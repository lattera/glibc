/* Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with the GNU C Library; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

#include <stdio.h>
#include <signal.h>

/* Get this configuration's defns of the signal numbers.  */
#define _SIGNAL_H 1
#include SIGNUM_H

/* Make a definition for sys_siglist.  */


#undef	HAVE_SYS_SIGLIST
#define HAVE_STRSIGNAL
#define HAVE_PSIGNAL
#define sys_siglist my_siglist	/* Avoid clash with signal.h.  */

#include "signame.c"


int
main()
{
  register int i;

  signame_init ();

  puts ("#include <stddef.h>\n");

  puts ("\n/* This is a list of all known signal numbers.  */");

  puts ("\nconst char *const _sys_siglist[] =\n  {");

  for (i = 0; i < NSIG; ++i)
    printf ("    \"%s\",\n", sys_siglist[i]);

  puts ("    NULL\n  };\n");

  puts ("weak_alias (_sys_siglist, sys_siglist)");
  exit (0);
}
