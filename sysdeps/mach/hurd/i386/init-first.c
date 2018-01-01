/* Initialization code run first thing by the ELF startup code.  For i386/Hurd.
   Copyright (C) 1995-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <assert.h>
#include <ctype.h>
#include <hurd.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sysdep.h>
#include <set-hooks.h>
#include "hurdstartup.h"
#include "hurdmalloc.h"		/* XXX */
#include "../locale/localeinfo.h"

#include <ldsodefs.h>
#include <fpu_control.h>

extern void __mach_init (void);
extern void __init_misc (int, char **, char **);
extern void __libc_global_ctors (void);

unsigned int __hurd_threadvar_max;
unsigned long int __hurd_threadvar_stack_offset;
unsigned long int __hurd_threadvar_stack_mask;

#ifndef SHARED
int __libc_enable_secure;
#endif
int __libc_multiple_libcs attribute_hidden = 1;

extern int __libc_argc attribute_hidden;
extern char **__libc_argv attribute_hidden;
extern char **_dl_argv;

extern void *(*_cthread_init_routine) (void) __attribute__ ((weak));
void (*_cthread_exit_routine) (int status) __attribute__ ((__noreturn__));

/* Things that want to be run before _hurd_init or much anything else.
   Importantly, these are called before anything tries to use malloc.  */
DEFINE_HOOK (_hurd_preinit_hook, (void));


/* We call this once the Hurd magic is all set up and we are ready to be a
   Posixoid program.  This does the same things the generic version does.  */
static void
posixland_init (int argc, char **argv, char **envp)
{
  __libc_multiple_libcs = &_dl_starting_up && !_dl_starting_up;

  /* Make sure we don't initialize twice.  */
  if (!__libc_multiple_libcs)
    {
      /* Set the FPU control word to the proper default value.  */
      __setfpucw (__fpu_control);
    }
  else
    {
      /* Initialize data structures so the additional libc can do RPCs.  */
      __mach_init ();
    }

  /* Save the command-line arguments.  */
  __libc_argc = argc;
  __libc_argv = argv;
  __environ = envp;

#ifndef SHARED
  _dl_non_dynamic_init ();
#endif
  __init_misc (argc, argv, envp);

  /* Initialize ctype data.  */
  __ctype_init ();

#if defined SHARED && !defined NO_CTORS_DTORS_SECTIONS
  __libc_global_ctors ();
#endif
}


static void
init1 (int argc, char *arg0, ...)
{
  char **argv = &arg0;
  char **envp = &argv[argc + 1];
  struct hurd_startup_data *d;

  while (*envp)
    ++envp;
  d = (void *) ++envp;

  /* If we are the bootstrap task started by the kernel,
     then after the environment pointers there is no Hurd
     data block; the argument strings start there.  */
  if ((void *) d == argv[0])
    {
#ifndef SHARED
      /* With a new enough linker (binutils-2.23 or better),
         the magic __ehdr_start symbol will be available and
         __libc_start_main will have done this that way already.  */
      if (_dl_phdr == NULL)
        {
          /* We may need to see our own phdrs, e.g. for TLS setup.
             Try the usual kludge to find the headers without help from
             the exec server.  */
          extern const void __executable_start;
          const ElfW(Ehdr) *const ehdr = &__executable_start;
          _dl_phdr = (const void *) ehdr + ehdr->e_phoff;
          _dl_phnum = ehdr->e_phnum;
          assert (ehdr->e_phentsize == sizeof (ElfW(Phdr)));
        }
#endif
      return;
    }

#ifndef SHARED
  __libc_enable_secure = d->flags & EXEC_SECURE;

  _dl_phdr = (ElfW(Phdr) *) d->phdr;
  _dl_phnum = d->phdrsz / sizeof (ElfW(Phdr));
  assert (d->phdrsz % sizeof (ElfW(Phdr)) == 0);
#endif

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

  if (d->portarray || d->intarray)
    /* Initialize library data structures, start signal processing, etc.  */
    _hurd_init (d->flags, argv,
		d->portarray, d->portarraysize,
		d->intarray, d->intarraysize);
}


static inline void
init (int *data)
{
  int argc = *data;
  char **argv = (void *) (data + 1);
  char **envp = &argv[argc + 1];
  struct hurd_startup_data *d;
  unsigned long int threadvars[_HURD_THREADVAR_MAX];

  /* Provide temporary storage for thread-specific variables on the
     startup stack so the cthreads initialization code can use them
     for malloc et al, or so we can use malloc below for the real
     threadvars array.  */
  memset (threadvars, 0, sizeof threadvars);
  threadvars[_HURD_THREADVAR_LOCALE] = (unsigned long int) &_nl_global_locale;
  __hurd_threadvar_stack_offset = (unsigned long int) threadvars;

  /* Since the cthreads initialization code uses malloc, and the
     malloc initialization code needs to get at the environment, make
     sure we can find it.  We'll need to do this again later on since
     switching stacks changes the location where the environment is
     stored.  */
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

  if (&_cthread_init_routine && _cthread_init_routine)
    {
      /* Initialize cthreads, which will allocate us a new stack to run on.  */
      int *newsp = (*_cthread_init_routine) ();
      struct hurd_startup_data *od;

      void switch_stacks (void);

      __libc_stack_end = newsp;

      /* Copy per-thread variables from that temporary
	 area onto the new cthread stack.  */
      memcpy (__hurd_threadvar_location_from_sp (0, newsp),
	      threadvars, sizeof threadvars);

      /* Copy the argdata from the old stack to the new one.  */
      newsp = memcpy (newsp - ((char *) &d[1] - (char *) data), data,
		      (char *) d - (char *) data);

#ifdef SHARED
      /* And readjust the dynamic linker's idea of where the argument
	 vector lives.  */
      assert (_dl_argv == argv);
      _dl_argv = (void *) (newsp + 1);
#endif

      /* Set up the Hurd startup data block immediately following
	 the argument and environment pointers on the new stack.  */
      od = ((void *) newsp + ((char *) d - (char *) data));
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
      /* The following expression would typically be written as
	 ``__builtin_return_address (0)''.  But, for example, GCC 4.4.6 doesn't
	 recognize that this read operation may alias the following write
	 operation, and thus is free to reorder the two, clobbering the
	 original return address.  */
      *--newsp = *((int *) __builtin_frame_address (0) + 1);
      /* GCC 4.4.6 also wants us to force loading *NEWSP already here.  */
      asm volatile ("# %0" : : "X" (*newsp));
      *((void **) __builtin_frame_address (0) + 1) = &switch_stacks;
      /* Force NEWSP into %eax and &init1 into %ecx, which are not restored
	 by function return.  */
      asm volatile ("# a %0 c %1" : : "a" (newsp), "c" (&init1));
    }
  else
    {
      /* We are not using cthreads, so we will have just a single allocated
	 area for the per-thread variables of the main user thread.  */
      unsigned long int *array;
      unsigned int i;
      int usercode;

      void call_init1 (void);

      array = malloc (__hurd_threadvar_max * sizeof (unsigned long int));
      if (array == NULL)
	__libc_fatal ("Can't allocate single-threaded thread variables.");

      /* Copy per-thread variables from the temporary array into the
	 newly malloc'd space.  */
      memcpy (array, threadvars, sizeof threadvars);
      __hurd_threadvar_stack_offset = (unsigned long int) array;
      for (i = _HURD_THREADVAR_MAX; i < __hurd_threadvar_max; ++i)
	array[i] = 0;

      /* The argument data is just above the stack frame we will unwind by
	 returning.  Mutate our own return address to run the code below.  */
      /* The following expression would typically be written as
	 ``__builtin_return_address (0)''.  But, for example, GCC 4.4.6 doesn't
	 recognize that this read operation may alias the following write
	 operation, and thus is free to reorder the two, clobbering the
	 original return address.  */
      usercode = *((int *) __builtin_frame_address (0) + 1);
      /* GCC 4.4.6 also wants us to force loading USERCODE already here.  */
      asm volatile ("# %0" : : "X" (usercode));
      *((void **) __builtin_frame_address (0) + 1) = &call_init1;
      /* Force USERCODE into %eax and &init1 into %ecx, which are not
	 restored by function return.  */
      asm volatile ("# a %0 c %1" : : "a" (usercode), "c" (&init1));
    }
}

/* These bits of inline assembler used to be located inside `init'.
   However they were optimized away by gcc 2.95.  */

/* The return address of `init' above, was redirected to here, so at
   this point our stack is unwound and callers' registers restored.
   Only %ecx and %eax are call-clobbered and thus still have the
   values we set just above.  Fetch from there the new stack pointer
   we will run on, and jmp to the run-time address of `init1'; when it
   returns, it will run the user code with the argument data at the
   top of the stack.  */
asm ("switch_stacks:\n"
     "	movl %eax, %esp\n"
     "	jmp *%ecx");

/* As in the stack-switching case, at this point our stack is unwound
   and callers' registers restored, and only %ecx and %eax communicate
   values from the lines above.  In this case we have stashed in %eax
   the user code return address.  Push it on the top of the stack so
   it acts as init1's return address, and then jump there.  */
asm ("call_init1:\n"
     "	push %eax\n"
     "	jmp *%ecx\n");


/* Do the first essential initializations that must precede all else.  */
static inline void
first_init (void)
{
  /* Initialize data structures so we can do RPCs.  */
  __mach_init ();

  RUN_HOOK (_hurd_preinit_hook, ());
}

#ifdef SHARED
/* This function is called specially by the dynamic linker to do early
   initialization of the shared C library before normal initializers
   expecting a Posixoid environment can run.  It gets called with the
   stack set up just as the user will see it, so it can switch stacks.  */

void
_dl_init_first (int argc, ...)
{
  first_init ();

  /* If we use ``__builtin_frame_address (0) + 2'' here, GCC gets confused.  */
  init (&argc);
}
#endif


#ifdef SHARED
/* The regular posixland initialization is what goes into libc's
   normal initializer.  */
/* NOTE!  The linker notices the magical name `_init' and sets the DT_INIT
   pointer in the dynamic section based solely on that.  It is convention
   for this function to be in the `.init' section, but the symbol name is
   the only thing that really matters!!  */
strong_alias (posixland_init, _init);

void
__libc_init_first (int argc, char **argv, char **envp)
{
  /* Everything was done in the shared library initializer, _init.  */
}
#else
strong_alias (posixland_init, __libc_init_first);


/* XXX This is all a crock and I am not happy with it.
   This poorly-named function is called by static-start.S,
   which should not exist at all.  */
void
_hurd_stack_setup (void)
{
  intptr_t caller = (intptr_t) __builtin_return_address (0);

  void doinit (intptr_t *data)
    {
      /* This function gets called with the argument data at TOS.  */
      void doinit1 (int argc, ...)
	{
	  /* If we use ``__builtin_frame_address (0) + 2'' here, GCC gets
	     confused.  */
	  init ((int *) &argc);
	}

      /* Push the user return address after the argument data, and then
	 jump to `doinit1' (above), so it is as if __libc_init_first's
	 caller had called `doinit1' with the argument data already on the
	 stack.  */
      *--data = caller;
      asm volatile ("movl %0, %%esp\n" /* Switch to new outermost stack.  */
		    "movl $0, %%ebp\n" /* Clear outermost frame pointer.  */
		    "jmp *%1" : : "r" (data), "r" (&doinit1) : "sp");
      /* NOTREACHED */
    }

  first_init ();

  _hurd_startup ((void **) __builtin_frame_address (0) + 2, &doinit);
}
#endif


/* This function is defined here so that if this file ever gets into
   ld.so we will get a link error.  Having this file silently included
   in ld.so causes disaster, because the _init definition above will
   cause ld.so to gain an init function, which is not a cool thing. */

void
_dl_start (void)
{
  abort ();
}
