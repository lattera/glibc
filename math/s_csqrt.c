/* Complex square root of double value.
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

__complex__ double
__csqrt (__complex__ double x)
{
  __complex__ double res;
  int rcls = fpclassify (__real__ x);
  int icls = fpclassify (__imag__ x);

  if (__builtin_expect (rcls <= FP_INFINITE || icls <= FP_INFINITE, 0))
    {
      if (icls == FP_INFINITE)
	{
	  __real__ res = HUGE_VAL;
	  __imag__ res = __imag__ x;
	}
      else if (rcls == FP_INFINITE)
	{
	  if (__real__ x < 0.0)
	    {
	      __real__ res = icls == FP_NAN ? __nan ("") : 0;
	      __imag__ res = __copysign (HUGE_VAL, __imag__ x);
	    }
	  else
	    {
	      __real__ res = __real__ x;
	      __imag__ res = (icls == FP_NAN
			      ? __nan ("") : __copysign (0.0, __imag__ x));
	    }
	}
      else
	{
	  __real__ res = __nan ("");
	  __imag__ res = __nan ("");
	}
    }
  else
    {
      if (__builtin_expect (icls == FP_ZERO, 0))
	{
	  if (__real__ x < 0.0)
	    {
	      __real__ res = 0.0;
	      __imag__ res = __copysign (__ieee754_sqrt (-__real__ x),
					 __imag__ x);
	    }
	  else
	    {
	      __real__ res = fabs (__ieee754_sqrt (__real__ x));
	      __imag__ res = __copysign (0.0, __imag__ x);
	    }
	}
      else if (__builtin_expect (rcls == FP_ZERO, 0))
	{
	  double r;
	  if (fabs (__imag__ x) >= 2.0 * DBL_MIN)
	    r = __ieee754_sqrt (0.5 * fabs (__imag__ x));
	  else
	    r = 0.5 * __ieee754_sqrt (2.0 * fabs (__imag__ x));

	  __real__ res = r;
	  __imag__ res = __copysign (r, __imag__ x);
	}
      else
	{
	  double d, r, s;
	  int scale = 0;

	  if (fabs (__real__ x) > DBL_MAX / 4.0)
	    {
	      scale = 1;
	      __real__ x = __scalbn (__real__ x, -2 * scale);
	      __imag__ x = __scalbn (__imag__ x, -2 * scale);
	    }
	  else if (fabs (__imag__ x) > DBL_MAX / 4.0)
	    {
	      scale = 1;
	      if (fabs (__real__ x) >= 4.0 * DBL_MIN)
		__real__ x = __scalbn (__real__ x, -2 * scale);
	      else
		__real__ x = 0.0;
	      __imag__ x = __scalbn (__imag__ x, -2 * scale);
	    }
	  else if (fabs (__real__ x) < DBL_MIN
		   && fabs (__imag__ x) < DBL_MIN)
	    {
	      scale = -(DBL_MANT_DIG / 2);
	      __real__ x = __scalbn (__real__ x, -2 * scale);
	      __imag__ x = __scalbn (__imag__ x, -2 * scale);
	    }

	  d = __ieee754_hypot (__real__ x, __imag__ x);
	  /* Use the identity   2  Re res  Im res = Im x
	     to avoid cancellation error in  d +/- Re x.  */
	  if (__real__ x > 0)
	    {
	      r = __ieee754_sqrt (0.5 * (d + __real__ x));
	      s = 0.5 * (__imag__ x / r);
	    }
	  else
	    {
	      s = __ieee754_sqrt (0.5 * (d - __real__ x));
	      r = fabs (0.5 * (__imag__ x / s));
	    }

	  if (scale)
	    {
	      r = __scalbn (r, scale);
	      s = __scalbn (s, scale);
	    }

	  __real__ res = r;
	  __imag__ res = __copysign (s, __imag__ x);
	}
    }

  return res;
}
weak_alias (__csqrt, csqrt)
#ifdef NO_LONG_DOUBLE
strong_alias (__csqrt, __csqrtl)
weak_alias (__csqrt, csqrtl)
#endif
