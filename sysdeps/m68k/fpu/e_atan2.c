/* Copyright (C) 1997 Free Software Foundation, Inc.
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

#define __LIBC_M81_MATH_INLINES
#include <math.h>
#include "math_private.h"

#ifndef SUFF
#define SUFF
#endif
#ifndef float_type
#define float_type double
#endif

#define __CONCATX(a,b) __CONCAT(a,b)

float_type
__CONCATX(__ieee754_atan2,SUFF) (float_type y, float_type x)
{
  float_type pi, pi_2, z;

  __asm ("fmovecr%.x %#0, %0" : "=f" (pi));
  __asm ("fscale%.w %#-1, %0" : "=f" (pi_2) : "0" (pi));
  if (x != x || y != y)
    z = x + y;
  else if (y == 0)
    {
      if (signbit (x))
	z = signbit (y) ? -pi : pi;
      else
	z = y;
    }
  else if (__m81_u(__CONCATX(__isinf,SUFF)) (x))
    {
      if (__m81_u(__CONCATX(__isinf,SUFF)) (y))
	{
	  float_type pi_4;
	  __asm ("fscale%.w %#-1, %0" : "=f" (pi_4) : "0" (pi_2));
	  z = x > 0 ? pi_4 : 3 * pi_4;
	}
      else
	z = x > 0 ? 0 : pi;
      if (signbit (y))
	z = -z;
    }
  else if (__m81_u(__CONCATX(__isinf,SUFF)) (y))
    z = y > 0 ? pi_2 : -pi_2;
  else if (x > 0)
    {
      if (y > 0)
	{
	  if (x > y)
	    z = __m81_u(__CONCATX(__atan,SUFF)) (y / x);
	  else
	    z = pi_2 - __m81_u(__CONCATX(__atan,SUFF)) (x / y);
	}
      else
	{
	  if (x > -y)
	    z = __m81_u(__CONCATX(__atan,SUFF)) (y / x);
	  else
	    z = -pi_2 - __m81_u(__CONCATX(__atan,SUFF)) (x / y);
	}
    }
  else
    {
      if (y < 0)
	{
	  if (-x > y)
	    z = -pi + __m81_u(__CONCATX(__atan,SUFF)) (y / x);
	  else
	    z = -pi_2 - __m81_u(__CONCATX(__atan,SUFF)) (x / y);
	}
      else
	{
	  if (-x > y)
	    z = pi + __m81_u(__CONCATX(__atan,SUFF)) (y / x);
	  else
	    z = pi_2 - __m81_u(__CONCATX(__atan,SUFF)) (x / y);
	}
    }
  return z;
}
