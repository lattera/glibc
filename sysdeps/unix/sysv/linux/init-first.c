/* Initialization code run first thing by the ELF startup code.  Linux version.
Copyright (C) 1995, 1996 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <sysdep.h>
#include <fpu_control.h>
#include <linux/personality.h>
#include "init-first.h"

extern void __libc_init (int, char **, char **);
extern void __libc_global_ctors (void);

/* The function is called from assembly stubs the compiler can't see.  */
static void init (void *) __attribute__ ((unused));

extern int __libc_is_static;
#ifdef PIC
weak_extern (__libc_is_static)
#endif

static void
init (void *data)
{
  extern int __personality (int);

  int argc = *(long *)data;
  char **argv = (char **)data + 1;
  char **envp = &argv[argc + 1];

#ifdef PIC
  if (&__libc_is_static != NULL)
#endif
    {
#ifdef PIC
      /* We must not call `personality' twice.  */
      if (__libc_is_static == 0)
#endif
	{
	  /* The `personality' system call takes one argument that
	     chooses the "personality", i.e. the set of system calls
	     and such.  We must make this call first thing to disable
	     emulation of some other system that might have been
	     enabled by default based on the executable format.  */
	  __personality (PER_LINUX);

	  /* Set the FPU control word to the proper default value.  */
	  __setfpucw (__fpu_control);
	}

      /* We set LIBC_IS_STATIC to a value > 0 for the static library
	 and < 0 for the shared library.  This information might be
	 useful for the running program but it is mainly necessary for
	 the above `if' statement.  */
#ifdef PIC
      __libc_is_static = -1;
#else
      __libc_is_static = 1;
#endif
    }

  __environ = envp;
  __libc_init (argc, argv, envp);

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

SYSDEP_CALL_INIT(__libc_init_first, init);

#endif
