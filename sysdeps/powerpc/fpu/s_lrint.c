/* Round floating-point to integer.  PowerPC version.
   Copyright (C) 1997 Free Software Foundation, Inc.
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

#include "math.h"

long int
__lrint (double x)
{
  union {
    double d;
    long int ll[2];
  } u;
  asm ("fctiw %0,%1" : "=f"(u.d) : "f"(x));
  return u.ll[1];
}
weak_alias (__lrint, lrint)

/* This code will also work for a 'float' argument.  */
asm ("\n\
	.globl __lrintf	\n\
	.globl lrintf	\n\
	.weak lrintf	\n\
	.set __lrintf,__lrint	\n\
	.set lrintf,__lrint	\n\
");

#ifdef NO_LONG_DOUBLE
strong_alias (__lrint, __lrintl)
weak_alias (__lrint, lrintl)
#endif
