/* Initialization code run first thing by the ELF startup code.  Stub version.
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

#include <unistd.h>

extern void __libc_init (int, char **, char **);

#ifdef PIC
void
__libc_init_first (void)
{
}
#endif

#ifdef PIC
/* NOTE!  The linker notices the magical name `_init' and sets the DT_INIT
   pointer in the dynamic section based solely on that.  It is convention
   for this function to be in the `.init' section, but the symbol name is
   the only thing that really matters!!  */
void _init
#else
void __libc_init_first
#endif
(int argc, char *arg0, ...)
{
  char **argv = &arg0, **envp = &argv[argc + 1];

  __environ = envp;
  __libc_init (argc, argv, envp);
}
