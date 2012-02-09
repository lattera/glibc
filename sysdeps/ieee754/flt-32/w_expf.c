/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gmail.com>, 2011.

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

#include <math.h>
#include <math_private.h>

static const float
o_threshold=  8.8722831726e+01,  /* 0x42b17217 */
u_threshold= -1.0397208405e+02;  /* 0xc2cff1b5 */


/* wrapper expf */
float
__expf (float x)
{
  if (__builtin_expect (x > o_threshold, 0))
    {
      if (_LIB_VERSION != _IEEE_)
	return __kernel_standard_f (x, x, 106);
    }
  else if (__builtin_expect (x < u_threshold, 0))
    {
      if (_LIB_VERSION != _IEEE_)
	return __kernel_standard_f (x, x, 107);
    }

  return __ieee754_expf (x);
}
hidden_def (__expf)
weak_alias (__expf, expf)
