/* Complex square root of long double value.
   Copyright (C) 1997-2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Based on an algorithm by Stephen L. Moshier <moshier@world.std.com>.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <complex.h>
#include <math.h>
#include <math_private.h>
#include <float.h>

__complex__ long double
__csqrtl (__complex__ long double x)
{
  __complex__ long double res;
  int rcls = fpclassify (__real__ x);
  int icls = fpclassify (__imag__ x);

  if (__builtin_expect (rcls <= FP_INFINITE || icls <= FP_INFINITE, 0))
    {
      if (icls == FP_INFINITE)
	{
	  __real__ res = HUGE_VALL;
	  __imag__ res = __imag__ x;
	}
      else if (rcls == FP_INFINITE)
	{
	  if (__real__ x < 0.0)
	    {
	      __real__ res = icls == FP_NAN ? __nanl ("") : 0;
	      __imag__ res = __copysignl (HUGE_VALL, __imag__ x);
	    }
	  else
	    {
	      __real__ res = __real__ x;
	      __imag__ res = (icls == FP_NAN
			      ? __nanl ("") : __copysignl (0.0, __imag__ x));
	    }
	}
      else
	{
	  __real__ res = __nanl ("");
	  __imag__ res = __nanl ("");
	}
    }
  else
    {
      if (__builtin_expect (icls == FP_ZERO, 0))
	{
	  if (__real__ x < 0.0)
	    {
	      __real__ res = 0.0;
	      __imag__ res = __copysignl (__ieee754_sqrtl (-__real__ x),
					  __imag__ x);
	    }
	  else
	    {
	      __real__ res = fabsl (__ieee754_sqrtl (__real__ x));
	      __imag__ res = __copysignl (0.0, __imag__ x);
	    }
	}
      else if (__builtin_expect (rcls == FP_ZERO, 0))
	{
	  long double r;
	  if (fabsl (__imag__ x) >= 2.0L * LDBL_MIN)
	    r = __ieee754_sqrtl (0.5L * fabsl (__imag__ x));
	  else
	    r = 0.5L * __ieee754_sqrtl (2.0L * fabsl (__imag__ x));

	  __real__ res = r;
	  __imag__ res = __copysignl (r, __imag__ x);
	}
      else
	{
	  long double d, r, s;
	  int scale = 0;

	  if (fabsl (__real__ x) > LDBL_MAX / 4.0L)
	    {
	      scale = 1;
	      __real__ x = __scalbnl (__real__ x, -2 * scale);
	      __imag__ x = __scalbnl (__imag__ x, -2 * scale);
	    }
	  else if (fabsl (__imag__ x) > LDBL_MAX / 4.0L)
	    {
	      scale = 1;
	      if (fabsl (__real__ x) >= 4.0L * LDBL_MIN)
		__real__ x = __scalbnl (__real__ x, -2 * scale);
	      else
		__real__ x = 0.0L;
	      __imag__ x = __scalbnl (__imag__ x, -2 * scale);
	    }
	  else if (fabsl (__real__ x) < LDBL_MIN
		   && fabsl (__imag__ x) < LDBL_MIN)
	    {
	      scale = -(LDBL_MANT_DIG / 2);
	      __real__ x = __scalbnl (__real__ x, -2 * scale);
	      __imag__ x = __scalbnl (__imag__ x, -2 * scale);
	    }

	  d = __ieee754_hypotl (__real__ x, __imag__ x);
	  /* Use the identity   2  Re res  Im res = Im x
	     to avoid cancellation error in  d +/- Re x.  */
	  if (__real__ x > 0)
	    {
	      r = __ieee754_sqrtl (0.5L * (d + __real__ x));
	      s = 0.5L * (__imag__ x / r);
	    }
	  else
	    {
	      s = __ieee754_sqrtl (0.5L * (d - __real__ x));
	      r = fabsl (0.5L * (__imag__ x / s));
	    }

	  if (scale)
	    {
	      r = __scalbnl (r, scale);
	      s = __scalbnl (s, scale);
	    }

	  __real__ res = r;
	  __imag__ res = __copysignl (s, __imag__ x);
	}
    }

  return res;
}
weak_alias (__csqrtl, csqrtl)
