/* Copyright (C) 1991, 1992, 1995 Free Software Foundation, Inc.
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


/* Make a definition for sys_errlist.  */

extern int sys_nerr;
extern char *sys_errlist[];

int
main ()
{
  register int i;

  puts ("#include \"ansidecl.h\"\n#include <stddef.h>\n");
  puts ("\n/* This is a list of all known `errno' codes.  */\n");

  puts ("#ifndef HAVE_WEAK_SYMBOLS");
  puts ("#define _sys_nerr\tsys_nerr");
  puts ("#define _sys_errlist\tsys_errlist");
  puts ("#endif");

  printf ("\nCONST int _sys_nerr = %d;\n\n", sys_nerr);
  puts ("CONST char *CONST _sys_errlist[] =\n  {");

  for (i = 0; i < sys_nerr; ++i)
    printf ("    \"%s\",\n", sys_errlist[i]);

  puts ("    NULL\n  };\n");

  puts ("weak_alias (_sys_errlist, sys_errlist)");
  puts ("weak_alias (_sys_nerr, sys_nerr)");

  exit (0);
}
