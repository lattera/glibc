/* Floating-point absolute value.  PowerPC version.
   Copyright (C) 1997-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

ENTRY(__fabs)
/* double [f1] fabs (double [f1] x); */
	fabs fp1,fp1
	blr
END(__fabs)

weak_alias (__fabs,fabs)

/* It turns out that it's safe to use this code even for single-precision.  */
strong_alias(__fabs,__fabsf)
weak_alias (__fabs,fabsf)

#ifdef NO_LONG_DOUBLE
weak_alias (__fabs,__fabsl)
weak_alias (__fabs,fabsl)
#endif
