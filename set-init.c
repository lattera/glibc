/* Copyright (C) 1991, 1992, 1994, 1995 Free Software Foundation, Inc.
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

#include <stdlib.h>
#include "set-hooks.h"

DEFINE_HOOK_RUNNER (__libc_subinit, __libc_subinit_runner,
		    (int argc, char **argv, char **envp), (argc, argv, envp))

void
__libc_init (argc, argv, envp)
     int argc;
     char **argv;
     char **envp;
{
  __libc_subinit_runner (argc, argv, envp);

#ifdef HAVE_ELF
  {
    /* These functions are defined in crti.o to run the .init and .fini
       sections, which are used for initializers in ELF.  */
    extern void _init __P ((void)), _fini __P ((void));
    atexit (_fini);		/* Arrange for _fini to run at exit.  */
    _init ();
  }
#endif
}
