/* Copyright (C) 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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

#ifdef	HAVE_GNU_LD

#include <ansidecl.h>
#include <string.h>

char *program_invocation_name;
char *program_invocation_short_name;

static void
DEFUN(set_progname, (argc, argv, envp),
      int argc AND char **argv AND char **envp)
{
  char *p;

  if (argv && argv[0])
    {
      program_invocation_name = argv[0];
      p = strrchr (argv[0], '/');
      if (p == NULL)
	program_invocation_short_name = argv[0];
      else
	program_invocation_short_name = p + 1;
    }
  else
    program_invocation_name = program_invocation_short_name = 0;

  (void) &set_progname;		/* Avoid "defined but not used" warning.  */
}

text_set_element (__libc_subinit, set_progname);

#endif
