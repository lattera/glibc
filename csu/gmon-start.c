/* Code to enable profiling at program startup.
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

#include <sys/types.h>
#include <sys/gmon.h>
#include <stdlib.h>
#include <unistd.h>

/* Beginning and end of our code segment.  */
extern void _start (), etext ();

/* These functions are defined in gmon/gmon.c;
   they do all the work of setting up and cleaning up profiling.  */
extern void monstartup (u_long, u_long);
extern void _mcleanup (void);

#ifndef HAVE_INITFINI
/* This function gets called at startup by the normal constructor
   mechanism.  We link this file together with start.o to produce gcrt1.o,
   so this constructor will be first in the list.  */

void __gmon_start__ (void) __attribute__ ((constructor));
#else
/* In ELF and COFF, we cannot use the normal constructor mechanism to call
   __gmon_start__ because gcrt1.o appears before crtbegin.o in the link.
   Instead crti.o calls it specially (see initfini.c).  */
#endif

void
__gmon_start__ (void)
{
  /* Start keeping profiling records.  */
  monstartup ((u_long) &_start, (u_long) &etext);

  /* Call _mcleanup before exitting; it will write out gmon.out from the
     collected data.  */
  atexit (&_mcleanup);
}

