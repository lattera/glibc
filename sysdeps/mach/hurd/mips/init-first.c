/* Initialization code run first thing by the ELF startup code.  For Mips/Hurd.
   Copyright (C) 1996,1997,1998,2000,01,02,03 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <hurd.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "hurdstartup.h"
#include "set-hooks.h"
#include "hurdmalloc.h"		/* XXX */

extern void __mach_init (void);
extern void __init_misc (int, char **, char **);
#ifdef USE_NONOPTION_FLAGS
extern void __getopt_clean_environment (char **);
#endif
#ifndef SHARED
extern void _dl_non_dynamic_init (void) internal_function;
#endif
extern void __libc_global_ctors (void);

unsigned int __hurd_threadvar_max;
unsigned long int __hurd_threadvar_stack_offset;
unsigned long int __hurd_threadvar_stack_mask;

int __libc_multiple_libcs attribute_hidden = 1;

int __libc_argc attribute_hidden;
char **__libc_argv attribute_hidden;

void *(*_cthread_init_routine) (void); /* Returns new SP to use.  */
void (*_cthread_exit_routine) (int status) __attribute__ ((__noreturn__));


/* Things that want to be run before _hurd_init or much anything else.
   Importantly, these are called before anything tries to use malloc.  */
DEFINE_HOOK (_hurd_preinit_hook, (void));

static void
init1 (int argc, char *arg0, ...)
{
  char **argv = &arg0;
  char **envp = &argv[argc + 1];
  struct hurd_startup_data *d;

  __libc_argc = argc;
  __libc_argv = argv;
  __environ = envp;
  while (*envp)
    ++envp;
  d = (void *) ++envp;

  /* If we are the bootstrap task started by the kernel,
     then after the environment pointers there is no Hurd
     data block; the argument strings start there.  */
  if ((void *) d != argv[0])
    {
      _hurd_init_dtable = d->dtable;
      _hurd_init_dtablesize = d->dtablesize;

      {
	/* Check if the stack we are now on is different from
	   the one described by _hurd_stack_{base,size}.  */

	char dummy;
	const vm_address_t newsp = (vm_address_t) &dummy;

	if (d->stack_size != 0 && (newsp < d->stack_base ||
				   newsp - d->stack_base > d->stack_size))
	  /* The new stack pointer does not intersect with the
	     stack the exec server set up for us, so free that stack.  */
	  __vm_deallocate (__mach_task_self (), d->stack_base, d->stack_size);
      }
    }

  if (__hurd_threadvar_stack_mask == 0)
    {
      /* We are not using cthreads, so we will have just a single allocated
	 area for the per-thread variables of the main user thread.  */
      unsigned long int i;
      __hurd_threadvar_stack_offset
	= (unsigned long int) malloc (__hurd_threadvar_max *
				      sizeof (unsigned long int));
      if (__hurd_threadvar_stack_offset == 0)
	__libc_fatal ("Can't allocate single-threaded per-thread variables.");
      for (i = 0; i < __hurd_threadvar_max; ++i)
	((unsigned long int *) __hurd_threadvar_stack_offset)[i] = 0;
    }

  if ((void *) d != argv[0] && (d->portarray || d->intarray))
    /* Initialize library data structures, start signal processing, etc.  */
    _hurd_init (d->flags, argv,
		d->portarray, d->portarraysize,
		d->intarray, d->intarraysize);

#ifndef SHARED
  _dl_non_dynamic_init ();
#endif
  __init_misc (argc, argv, __environ);

#ifdef USE_NONOPTION_FLAGS
  /* This is a hack to make the special getopt in GNU libc working.  */
  __getopt_clean_environment (envp);
#endif

#ifdef SHARED
  __libc_global_ctors ();
#endif

  (void) &init1;
}

static void *
__init (int *data)
{
  int argc = *data;
  char **argv = (void *) (data + 1);
  char **envp = &argv[argc + 1];
  struct hurd_startup_data *d;

  __environ = envp;
  while (*envp)
    ++envp;
  d = (void *) ++envp;

  /* The user might have defined a value for this, to get more variables.
     Otherwise it will be zero on startup.  We must make sure it is set
     properly before before cthreads initialization, so cthreads can know
     how much space to leave for thread variables.  */
  if (__hurd_threadvar_max < _HURD_THREADVAR_MAX)
    __hurd_threadvar_max = _HURD_THREADVAR_MAX;


  /* After possibly switching stacks, call `init1' (above) with the user
     code as the return address, and the argument data immediately above
     that on the stack.  */

  if (_cthread_init_routine)
    {
      /* Initialize cthreads, which will allocate us a new stack to run on.  */
      void *newsp = (*_cthread_init_routine) ();
      struct hurd_startup_data *od;

      /* Copy the argdata from the old stack to the new one.  */
      newsp = memcpy (newsp - ((char *) &d[1] - (char *) data), data,
		      (char *) d - (char *) data);

      /* Set up the Hurd startup data block immediately following
	 the argument and environment pointers on the new stack.  */
      od = (newsp + ((char *) d - (char *) data));
      if ((void *) argv[0] == d)
	/* We were started up by the kernel with arguments on the stack.
	   There is no Hurd startup data, so zero the block.  */
	memset (od, 0, sizeof *od);
      else
	/* Copy the Hurd startup data block to the new stack.  */
	*od = *d;

      /* Push the user code address on the top of the new stack.  It will
	 be the return address for `init1'; we will jump there with NEWSP
	 as the stack pointer.  */
      return newsp;
    }

  /* The argument data is just above the stack frame we will unwind by
     returning.  */
  return (void *) data;

  (void) &__init;
}

#ifdef SHARED
/* This function is called to initialize the shared C library.
   It is called just before the user _start code from mips/elf/start.S,
   with the stack set up as that code gets it.  */

/* NOTE!  The linker notices the magical name `_init' and sets the DT_INIT
   pointer in the dynamic section based solely on that.  It is convention
   for this function to be in the `.init' section, but the symbol name is
   the only thing that really matters!!  */
/*void _init (int argc, ...) __attribute__ ((unused, section (".init")));*/

#if __mips64
asm ("\
	.section .init,\"ax\",@progbits\n\
	.align 3\n\
	.globl _init\n\
	.type _init,@function\n\
	.ent _init\n\
_init:\n\
	.set noreorder\n\
	.cpload $25\n\
	.set reorder\n\
	dsubu $29, 8*8\n\
	.cprestore 6*8\n\
	sd $16, 4*8($29)\n\
	sd $31, 5*8($29)\n\
	jal preinit\n\
	sd $28, 6*8($29)\n\
	move $16, $29 # Save the old stack pointer to s0 ($16)\n\
	daddu $4, $29, 4*8\n\
	jal __init\n\
	# Restore saved registers from the old stack.\n\
	ld $28, 6*8($16)\n\
	ld $31, 5*8($16)\n\
	ld $16, 4*8($16)\n\
	move $29, $2 # set new sp to SP\n\
call_init1:\n\
	ld $4, 0($29)\n\
	ld $5, 1*8($29)\n\
	ld $6, 2*8($29)\n\
	ld $7, 3*8($29)\n\
	dla $25, init1\n\
	jr $25\n\
	.end _init\n\
	.text\n\
");
#else
asm ("\
	.section .init,\"ax\",@progbits\n\
	.align 2\n\
	.globl _init\n\
	.type _init,@function\n\
	.ent _init\n\
_init:\n\
	.set noreorder\n\
	.cpload $25\n\
	.set reorder\n\
	subu $29, 32\n\
	.cprestore 24\n\
	sw $16, 16($29)\n\
	sw $31, 20($29)\n\
	jal preinit\n\
	sw $28, 24($29)\n\
	move $16, $29 # Save the old stack pointer to s0 ($16)\n\
	addu $4, $29, 32\n\
	jal __init\n\
	# Restore saved registers from the old stack.\n\
	lw $28, 24($16)\n\
	lw $31, 20($16)\n\
	lw $16, 16($16)\n\
	move $29, $2 # set new sp to SP\n\
call_init1:\n\
	lw $4, 0($29)\n\
	lw $5, 4($29)\n\
	lw $6, 8($29)\n\
	lw $7, 12($29)\n\
	la $25, init1\n\
	jr $25\n\
	.end _init\n\
	.text\n\
");
#endif

static void
preinit (void)
{
  /* Initialize data structures so we can do RPCs.  */
  __mach_init ();

  RUN_HOOK (_hurd_preinit_hook, ());

  (void) &preinit;
}

void __libc_init_first (int argc, ...)
{
}
#endif

#ifndef SHARED
/* An assembler code wrapping c function __init.  */
#ifdef __mips64
asm ("\
	.text\n\
	.align 3\n\
init:\n\
	dsubu $29, 8*8\n\
	sd $16, 4*8($29)\n\
	sd $31, 5*8($29)\n\
	move $16, $29\n\
	jal __init\n\
	ld $31, 5*8($16)\n\
	ld $16, 4*8($16)\n\
	move $29, $2 # set new sp to SP\n\
call_init1:\n\
	ld $4, 0($29)\n\
	ld $5, 1*8($29)\n\
	ld $6, 2*8($29)\n\
	ld $7, 3*8($29)\n\
	dla $25, init1\n\
	jr $25\n\
");
#else
asm ("\
	.text\n\
	.align 2\n\
init:\n\
	subu $29, 32\n\
	sw $16, 16($29)\n\
	sw $31, 20($29)\n\
	move $16, $29\n\
	jal __init\n\
	lw $31, 20($16)\n\
	lw $16, 16($16)\n\
	move $29, $2 # set new sp to SP\n\
call_init1:\n\
	lw $4, 0($29)\n\
	lw $5, 4($29)\n\
	lw $6, 8($29)\n\
	lw $7, 12($29)\n\
	la $25, init1\n\
	jr $25\n\
");
#endif

/* An assembler code wrapping c function ___libc_init_first.
   ___libc_init_first does an RPC call to flush cache to put doinit
   function on the stack, so we should call __mach_init first in
   this wrap. */
#ifdef __mips64
asm ("\
	.text\n\
	.align 3\n\
	.globl __libc_init_first\n\
__libc_init_first:\n\
	dsubu $29, 8\n\
	sd $31, 0($29)\n\
	jal __mach_init\n\
	ld $4, 0($29)\n\
	ld $5, 1*8($29)\n\
	ld $6, 2*8($29)\n\
	ld $7, 3*8($29)\n\
	j ___libc_init_first\n\
");
#else
asm ("\
	.text\n\
	.align 2\n\
	.globl __libc_init_first\n\
__libc_init_first:\n\
	subu $29, 4\n\
	sw $31, 0($29)\n\
	jal __mach_init\n\
	lw $4, 0($29)\n\
	lw $5, 4($29)\n\
	lw $6, 8($29)\n\
	lw $7, 12($29)\n\
	j ___libc_init_first\n\
");
#endif

static void
___libc_init_first (int return_addr, int argc, ...)
{
  void doinit (int *data)
    {
#if 0
      /* This function gets called with the argument data at TOS.  */
      void doinit1 (int argc, ...)
	{
	  init (&argc);
	}
#endif
      extern void init (int *data);

      /* Push the user return address after the argument data, and then
	 jump to `doinit1' (above), so it is as if __libc_init_first's
	 caller had called `init' with the argument data already on the
	 stack.  */
      *--data = return_addr;

#ifdef __mips64
      asm volatile ("ld $31, 0(%0)\n" /* Load the original return address.  */
		    "daddu $29, %0, 8\n" /* Switch to new outermost stack.  */
		    "move $4, $29\n"
		    "jr %1" : : "r" (data), "r" (&init));
#else
      asm volatile ("lw $31, 0(%0)\n" /* Load the original return address.  */
		    "addu $29, %0, 4\n" /* Switch to new outermost stack.  */
		    "move $4, $29\n"
		    "jr %1" : : "r" (data), "r" (&init));
#endif
      /* NOTREACHED */
    }

#if 0
  /* Initialize data structures so we can do RPCs.  */
  __mach_init ();
#endif

  RUN_HOOK (_hurd_preinit_hook, ());

  _hurd_startup ((void **) &argc, &doinit);

  (void) &___libc_init_first;
}
#endif
