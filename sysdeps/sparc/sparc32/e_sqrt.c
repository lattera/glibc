/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
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

#include <errno.h>
#include <math.h>

#ifndef	__GNUC__
  #error This file uses GNU C extensions; you must compile with GCC.
#endif

/* Return the square root of X.  */
double
__ieee754_sqrt (double x)
{
  register double result;
  asm ("fsqrtd %1, %0" : "=f" (result) : "f" (x));
  return result;
}
strong_alias (__ieee754_sqrt, __sqrt_finite)
