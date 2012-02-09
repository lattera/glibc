/* Square root of floating point number.
   Copyright (C) 2002, 2011 Free Software Foundation, Inc.
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

#include <math_private.h>

#undef __ieee754_sqrtf
float
__ieee754_sqrtf (float x)
{
  float res;

  asm ("sqrtss %1, %0" : "=x" (res) : "xm" (x));

  return res;
}
strong_alias (__ieee754_sqrtf, __sqrtf_finite)
