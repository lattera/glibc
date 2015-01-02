/* Copyright (C) 1997-2015 Free Software Foundation, Inc.
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

#if !defined _MATH_H && !defined _COMPLEX_H
# error "Never use <bits/mathdef.h> directly; include <math.h> instead"
#endif


/* FIXME! This file describes properties of the compiler, not the machine;
   it should not be part of libc!

   FIXME! This file does not deal with the -fshort-double option of
   gcc! */

#if defined __USE_ISOC99 && defined _MATH_H && !defined _MATH_H_MATHDEF
# define _MATH_H_MATHDEF	1

/* SH has both `float' and `double' arithmetic.  */
typedef float float_t;
typedef double double_t;

/* The values returned by `ilogb' for 0 and NaN respectively.  */
# define FP_ILOGB0	0x80000001
# define FP_ILOGBNAN	0x7fffffff

#endif	/* ISO C99 */

#ifndef __NO_LONG_DOUBLE_MATH
/* Signal that we do not really have a `long double'.  The disables the
   declaration of all the `long double' function variants.  */
# define __NO_LONG_DOUBLE_MATH	1
#endif
