/* Complex square root of long double value.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Based on an algorithm by Stephen L. Moshier <moshier@world.std.com>.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <complex.h>
#include <math.h>

#include "math_private.h"


__complex__ long double
__csqrtl (__complex__ long double x)
{
  __complex__ long double res;
  int rcls = fpclassify (__real__ x);
  int icls = fpclassify (__imag__ x);

  if (rcls <= FP_INFINITE || icls <= FP_INFINITE)
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
      if (icls == FP_ZERO)
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
      else if (rcls == FP_ZERO)
	{
	  long double r = __ieee754_sqrtl (0.5 * fabsl (__imag__ x));

	  __real__ res = __copysignl (r, __imag__ x);
	  __imag__ res = r;
	}
      else
	{
	  __complex__ long double q;
	  long double t, r;

	  if (fabsl (__imag__ x) < 2.0e-4 * fabsl (__real__ x))
	    t = 0.25 * __imag__ x * (__imag__ x / __real__ x);
	  else
	    t = 0.5 * (__ieee754_hypotl (__real__ x, __imag__ x) - __real__ x);

	  r = __ieee754_sqrtl (t);

	  __real__ q = __imag__ x / (2.0 * r);
	  __imag__ q = r;

	  /* Heron iteration in complex arithmetic.  */
	  res = 0.5 * (q + q / x);
	}
    }

  return res;
}
weak_alias (__csqrtl, csqrtl)
