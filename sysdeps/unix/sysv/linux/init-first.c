/* Initialization code run first thing by the ELF startup code.  Linux version.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <unistd.h>
#include <sysdep.h>
#include <fpu_control.h>
#include <linux/personality.h>
#include <init-first.h>
#include <sys/types.h>

extern void __libc_init_secure (void);
extern void __libc_init (int, char **, char **);
extern void __libc_global_ctors (void);

/* The function is called from assembly stubs the compiler can't see.  */
static void init (int, char **, char **) __attribute__ ((unused));

extern int _dl_starting_up;
weak_extern (_dl_starting_up)

extern fpu_control_t _dl_fpu_control;
extern int _dl_fpu_control_set;

/* Set nonzero if we have to be prepared for more then one libc being
   used in the process.  Safe assumption if initializer never runs.  */
int __libc_multiple_libcs = 1;

/* Remember the command line argument and enviroment contents for
   later calls of initializers for dynamic libraries.  */
int __libc_argc;
char **__libc_argv;


static void
init (int argc, char **argv, char **envp)
{
  extern int __personality (int);
  extern void __getopt_clean_environment (char **);

  /* We must not call `personality' twice.  */
  if (!__libc_multiple_libcs)
    {
      /* The `personality' system call takes one argument that chooses
	 the "personality", i.e. the set of system calls and such.  We
	 must make this call first thing to disable emulation of some
	 other system that might have been enabled by default based on
	 the executable format.  */
      __personality (PER_LINUX);

      /* Set the FPU control word to the proper default value if the
	 kernel would use a different value.  (In a static program we
	 don't have this information.)  */
#ifdef PIC
      if (__fpu_control != _dl_fpu_control)
#endif
	__setfpucw (__fpu_control);
    }

  /* Save the command-line arguments.  */
  __libc_argc = argc;
  __libc_argv = argv;
  __environ = envp;

#ifndef PIC
  __libc_init_secure ();
#endif

  __libc_init (argc, argv, envp);

  /* This is a hack to make the special getopt in GNU libc working.  */
  __getopt_clean_environment (envp);

#ifdef PIC
  __libc_global_ctors ();
#endif
}

#ifdef PIC

SYSDEP_CALL_INIT(_init, init);

void
__libc_init_first (void)
{
}

#else
void
__libc_init_first (int argc, char **argv, char **envp)
{
  init (argc, argv, envp);
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
