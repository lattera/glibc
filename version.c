/* Copyright (C) 1991, 92, 93, 94, 95, 96 Free Software Foundation, Inc.
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

#include <stdio.h>

#include "version.h"
const char __libc_release[] = RELEASE;
const char __libc_version[] = VERSION;

void
__libc_print_version (void)
{
  printf ("GNU C Library %s release version %s, by Roland McGrath et al.\n",
	  __libc_release, __libc_version);
#ifdef	__VERSION__
  printf ("Compiled by GNU CC version %s.\n", __VERSION__);
#endif
  puts ("\
Copyright (C) 1992, 93, 94, 95, 96 Free Software Foundation, Inc.\n\
This is free software; see the source for copying conditions.\n\
There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A\n\
PARTICULAR PURPOSE.");
}

#ifdef HAVE_ELF
/* This function is the entry point for the shared object.
   Running the library as a program will get here.  */

#include <stdlib.h>

void
__libc_main (void)
{
  __libc_print_version ();
  exit (0);
}
#endif

/*
   Local Variables:
   version-control: never
   End:
*/
