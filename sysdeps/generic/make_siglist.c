/* Copyright (C) 1991, 92, 93, 94, 95, 97 Free Software Foundation, Inc.
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

#include <stdio.h>

/* Include signal.h now so that we can safely reinclude it again in
   signame.c later on.  We completely override the definitions, we
   just have to be sure that the include guard in signal.h keeps it
   from redefining the signal values.  */
#include <signal.h>

/* Get this configuration's defns of the signal numbers.  */
#undef	_SIGNAL_H
#define _SIGNAL_H 1
#include SIGNUM_H

/* Make a definition for sys_siglist.  */


#undef	HAVE_SYS_SIGLIST
#define HAVE_STRSIGNAL
#define HAVE_PSIGNAL
#define sys_siglist my_siglist	/* Avoid clash with signal.h.  */

#undef NSIG
#define NSIG _NSIG	/* make sure that the value from SIGNUM_H is used.  */

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
