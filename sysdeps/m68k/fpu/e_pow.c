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
__CONCATX(__ieee754_pow,SUFF) (float_type x, float_type y)
{
  float_type z;
  float_type ax;

  if (y == 0.0)
    return 1.0;
  if (x != x || y != y)
    return x + y;

  if (__m81_u(__CONCATX(__isinf,SUFF)) (y))
    {
      ax = __CONCATX(fabs,SUFF) (x);
      if (ax == 1)
	return y - y;
      if (ax > 1)
	return y > 0 ? y : 0;
      else
	return y < 0 ? -y : 0;
    }

  if (__CONCATX(fabs,SUFF) (y) == 1)
    return y > 0 ? x : 1 / x;

  if (y == 2)
    return x * x;
  if (y == 0 && x >= 0)
    return __m81_u(__CONCATX(__ieee754_sqrt,SUFF)) (x);

  if (x == 10.0)
    {
      __asm ("ftentox%.x %1, %0" : "=f" (z) : "f" (y));
      return z;
    }
  if (x == 2.0)
    {
      __asm ("ftwotox%.x %1, %0" : "=f" (z) : "f" (y));
      return z;
    }

  ax = __CONCATX(fabs,SUFF) (x);
  if (__m81_u(__CONCATX(__isinf,SUFF)) (x) || x == 0 || ax == 1)
    {
      z = ax;
      if (y < 0)
	z = 1 / z;
      if (signbit (x))
	{
	  float_type temp = __m81_u (__CONCATX(__rint,SUFF)) (y);
	  if (y != temp)
	    {
	      if (x == -1)
		z = (z - z) / (z - z);
	    }
	  else
	    {
	      if (sizeof (float_type) == sizeof (float))
		{
		  long i = (long) y;
		  if (i & 1)
		    z = -z;
		}
	      else
		{
		  long long i = (long long) y;
		  if ((float_type) i == y && i & 1)
		    z = -z;
		}
	    }
	}
      return z;
    }

  if (x < 0.0)
    {
      float_type temp = __m81_u (__CONCATX(__rint,SUFF)) (y);
      if (y == temp)
	{
	  long long i = (long long) y;
	  z = (__m81_u(__CONCATX(__ieee754_exp,SUFF))
	       (y * __m81_u(__CONCATX(__ieee754_log,SUFF)) (-x)));
	  if (sizeof (float_type) == sizeof (float))
	    {
	      long i = (long) y;
	      if (i & 1)
		z = -z;
	    }
	  else
	    {
	      /* If the conversion to long long was inexact assume that y
		 is an even integer.  */
	      if ((float_type) i == y && i & 1)
		z = -z;
	    }
	}
      else
	z = (x - x) / (x - x);
    }
  else
    z = (__m81_u(__CONCATX(__ieee754_exp,SUFF))
	 (y * __m81_u(__CONCATX(__ieee754_log,SUFF)) (x)));
  return z;
}
