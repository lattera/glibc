/* Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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

#include <ansidecl.h>
#include <stdio.h>

CONST char __libc_release[] = "alpha";
CONST char __libc_version[] = "1.09.5";

void
DEFUN_VOID(__libc_print_version)
{
  printf ("GNU C Library %s release version %s, by Roland McGrath.",
	  __libc_release, __libc_version);
#ifdef	__VERSION__
  printf ("Compiled by GNU CC version %s.\n", __VERSION__);
#endif
  puts ("\
Copyright (C) 1992, 1993, 1994, 1995 Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.\n\
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A\n\
PARTICULAR PURPOSE.");
}

/*
   Local Variables:
   version-control: never
   End:
*/
