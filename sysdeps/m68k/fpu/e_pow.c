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

#define CONCATX(a,b) __CONCAT(a,b)
#define s(name) CONCATX(name,SUFF)
#define m81(func) __m81_u(s(func))

float_type
s(__ieee754_pow) (float_type x, float_type y)
{
  float_type z;
  float_type ax;

  if (y == 0.0)
    return 1.0;
  if (x != x || y != y)
    return x + y;

  if (m81(__isinf) (y))
    {
      ax = s(fabs) (x);
      if (ax == 1)
	return 0.0/0.0;
      if (ax > 1)
	return y > 0 ? y : 0;
      else
	return y < 0 ? -y : 0;
    }

  if (s(fabs) (y) == 1)
    return y > 0 ? x : 1 / x;

  if (y == 2)
    return x * x;
  if (y == 0.5 && x >= 0)
    return m81(__ieee754_sqrt) (x);

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

  ax = s(fabs) (x);
  if (m81(__isinf) (x) || x == 0 || ax == 1)
    {
      z = ax;
      if (y < 0)
	z = 1 / z;
      if (m81(__signbit) (x))
	{
	  if (y != m81(__rint) (y))
	    {
	      if (x == -1)
		z = 0.0/0.0;
	    }
	  else
	    goto maybe_negate;
	}
      return z;
    }

  if (x < 0.0)
    {
      if (y == m81(__rint) (y))
	{
	  z = m81(__ieee754_exp) (y * m81(__ieee754_log) (-x));
	maybe_negate:
	  /* We always use the long double format, since y is already in
	     this format and rounding won't change the result.  */
	  {
	    int32_t exponent;
	    u_int32_t i0, i1;
	    GET_LDOUBLE_WORDS (exponent, i0, i1, y);
	    exponent = (exponent & 0x7fff) - 0x3fff;
	    if (exponent <= 31
		? i0 & (1 << (31 - exponent))
		: (exponent <= 63
		   && i1 & (1 << (63 - exponent))))
	      z = -z;
	  }
	}
      else
	z = 0.0/0.0;
    }
  else
    z = m81(__ieee754_exp) (y * m81(__ieee754_log) (x));
  return z;
}
