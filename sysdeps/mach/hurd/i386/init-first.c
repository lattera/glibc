/* Initialization code run first thing by the ELF startup code.  For i386/Hurd.
Copyright (C) 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <hurd.h>
#include <stdio.h>
#include <unistd.h>
#include "hurdstartup.h"
#include "set-hooks.h"
#include "hurdmalloc.h"		/* XXX */

extern void __mach_init (void);
extern void __libc_init (int, char **, char **);

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

  __libc_init (argc, argv, __environ);
}

static void 
init (int *data, int retaddr)
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

  if (_cthread_init_routine)
    {
      /* Initialize cthreads, which will allocate us a new stack to run on.  */
      void *newsp = (*_cthread_init_routine) ();
      /* Copy the argdata from the old stack to the new one.  */
      newsp = memcpy (newsp - ((char *) &d[1] - (char *) data), data,
		      (char *) &d[1] - (char *) data);
      data = newsp;
    }

  /* Call `init1' (above) with the user code as the return address,
     and the argument data immediately above that on the stack.  */
  *--data = retaddr;
  asm volatile ("movl %0, %%esp; jmp %*%1" : : "g" (data), "r" (&init1));
}  


#ifdef PIC
/* This function is called to initialize the shared C library.
   It is called just before the user _start code from i386/elf/start.S,
   with the stack set up as that code gets it.  */

static void soinit (int argc, ...) __attribute__ ((unused, section (".init")));

static void
soinit (int argc, ...)
{
  /* Initialize data structures so we can do RPCs.  */
  __mach_init ();

  RUN_HOOK (_hurd_preinit_hook, ());
  
  init (&argc, (&argc)[-1]);

  (void) &soinit;		/* Avoid gcc optimizing this fn out.  */
}
#endif


void
__libc_init_first (int argc, ...)
{
#ifndef PIC
  void doinit (int *data)
    {
      init (data, (&argc)[-1]);
    }

  /* Initialize data structures so we can do RPCs.  */
  __mach_init ();

  RUN_HOOK (_hurd_preinit_hook, ());
  
  _hurd_startup ((void **) &argc, &doinit);
#endif
}
