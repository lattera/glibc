/* Initialization code run first thing by the ELF startup code.  Alpha/Hurd.
   Copyright (C) 1995,96,97,98,99,2000,01,02,03 Free Software Foundation, Inc.
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

#include <assert.h>
#include <hurd.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sysdep.h>
#include <set-hooks.h>
#include "hurdstartup.h"
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

#ifndef SHARED
int __libc_enable_secure;
#endif
int __libc_multiple_libcs attribute_hidden = 1;

extern int __libc_argc attribute_hidden;
extern char **__libc_argv attribute_hidden;
extern char **_dl_argv;

void *(*_cthread_init_routine) (void); /* Returns new SP to use.  */
void (*_cthread_exit_routine) (int status) __attribute__ ((__noreturn__));

/* Things that want to be run before _hurd_init or much anything else.
   Importantly, these are called before anything tries to use malloc.  */
DEFINE_HOOK (_hurd_preinit_hook, (void));


/* We call this once the Hurd magic is all set up and we are ready to be a
   Posixoid program.  This does the same things the generic version does.  */
static void
posixland_init (int argc, char **argv, char **envp)
{
  __libc_argc = argc;
  __libc_argv = argv;
  __environ = envp;

#ifndef SHARED
  _dl_non_dynamic_init ();
#endif
  __init_misc (argc, argv, envp);

#ifdef USE_NONOPTION_FLAGS
  /* This is a hack to make the special getopt in GNU libc working.  */
  __getopt_clean_environment (envp);
#endif

#ifdef SHARED
  __libc_global_ctors ();
#endif
}


static void
init1 (intptr_t *data)
{
  int argc = (intptr_t) *data;
  char **argv = (char **) &data[1];
  char **envp = &argv[argc + 1];
  struct hurd_startup_data *d;

  while (*envp)
    ++envp;
  d = (void *) ++envp;

  /* If we are the bootstrap task started by the kernel,
     then after the environment pointers there is no Hurd
     data block; the argument strings start there.  */
  /* OSF Mach starts the bootstrap task with argc == 0.
     XXX This fails if a non-bootstrap task gets started
     with argc == 0.  */
  if (argc && (void *) d != argv[0])
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

  if ((void *) d != argv[0] && (d->portarray || d->intarray))
    /* Initialize library data structures, start signal processing, etc.  */
    _hurd_init (d->flags, argv,
		d->portarray, d->portarraysize,
		d->intarray, d->intarraysize);

#ifndef SHARED
  __libc_enable_secure = d->flags & EXEC_SECURE;
#endif
}


static inline void
init (intptr_t *data)
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

  if (_cthread_init_routine)
    {
      /* Initialize cthreads, which will allocate us a new stack to run on.  */
      void *newsp = (*_cthread_init_routine) ();
      struct hurd_startup_data *od;

      void switch_stacks (void);

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
      _dl_argv = (void *) ((int *) newsp + 1);
#endif

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

      /*
         Force NEWSP into sp and &init1 into pv, then branch to pv (call init1).
       */
      asm volatile ("lda $30,0(%0); lda $27,0(%1); jsr $26,($27)"
		    : : "r" (newsp), "r" (&init1));
    }
  else
    {
      /* We are not using cthreads, so we will have just a single allocated
	 area for the per-thread variables of the main user thread.  */
      unsigned long int *array;
      unsigned int i;

      array = malloc (__hurd_threadvar_max * sizeof (unsigned long int));
      if (array == NULL)
	__libc_fatal ("Can't allocate single-threaded thread variables.");

      /* Copy per-thread variables from the temporary array into the
	 newly malloc'd space.  */
      memcpy (array, threadvars, sizeof threadvars);
      __hurd_threadvar_stack_offset = (unsigned long int) array;
      for (i = _HURD_THREADVAR_MAX; i < __hurd_threadvar_max; ++i)
	array[i] = 0;

      init1 (data);
    }
}


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
_dl_init_first (intptr_t argc, ...)
{
  first_init ();

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


void
_hurd_stack_setup (volatile intptr_t argc, ...)
{
  first_init ();

  _hurd_startup ((void **) &argc, &init);
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
