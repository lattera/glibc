/* Complex sine hyperbole function for long double.
   Copyright (C) 1997-2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
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
#include <fenv.h>
#include <math.h>
#include <math_private.h>
#include <float.h>

__complex__ long double
__csinhl (__complex__ long double x)
{
  __complex__ long double retval;
  int negate = signbit (__real__ x);
  int rcls = fpclassify (__real__ x);
  int icls = fpclassify (__imag__ x);

  __real__ x = fabsl (__real__ x);

  if (__builtin_expect (rcls >= FP_ZERO, 1))
    {
      /* Real part is finite.  */
      if (__builtin_expect (icls >= FP_ZERO, 1))
	{
	  /* Imaginary part is finite.  */
	  const int t = (int) ((LDBL_MAX_EXP - 1) * M_LN2l);
	  long double sinix, cosix;

	  if (__builtin_expect (icls != FP_SUBNORMAL, 1))
	    {
	      __sincosl (__imag__ x, &sinix, &cosix);
	    }
	  else
	    {
	      sinix = __imag__ x;
	      cosix = 1.0;
	    }

	  if (fabsl (__real__ x) > t)
	    {
	      long double exp_t = __ieee754_expl (t);
	      long double rx = fabsl (__real__ x);
	      if (signbit (__real__ x))
		cosix = -cosix;
	      rx -= t;
	      sinix *= exp_t / 2.0L;
	      cosix *= exp_t / 2.0L;
	      if (rx > t)
		{
		  rx -= t;
		  sinix *= exp_t;
		  cosix *= exp_t;
		}
	      if (rx > t)
		{
		  /* Overflow (original real part of x > 3t).  */
		  __real__ retval = LDBL_MAX * cosix;
		  __imag__ retval = LDBL_MAX * sinix;
		}
	      else
		{
		  long double exp_val = __ieee754_expl (rx);
		  __real__ retval = exp_val * cosix;
		  __imag__ retval = exp_val * sinix;
		}
	    }
	  else
	    {
	      __real__ retval = __ieee754_sinhl (__real__ x) * cosix;
	      __imag__ retval = __ieee754_coshl (__real__ x) * sinix;
	    }

	  if (negate)
	    __real__ retval = -__real__ retval;
	}
      else
	{
	  if (rcls == FP_ZERO)
	    {
	      /* Real part is 0.0.  */
	      __real__ retval = __copysignl (0.0, negate ? -1.0 : 1.0);
	      __imag__ retval = __nanl ("") + __nanl ("");

	      if (icls == FP_INFINITE)
		feraiseexcept (FE_INVALID);
	    }
	  else
	    {
	      __real__ retval = __nanl ("");
	      __imag__ retval = __nanl ("");

	      feraiseexcept (FE_INVALID);
	    }
	}
    }
  else if (__builtin_expect (rcls == FP_INFINITE, 1))
    {
      /* Real part is infinite.  */
      if (__builtin_expect (icls > FP_ZERO, 1))
	{
	  /* Imaginary part is finite.  */
	  long double sinix, cosix;

	  if (__builtin_expect (icls != FP_SUBNORMAL, 1))
	    {
	      __sincosl (__imag__ x, &sinix, &cosix);
	    }
	  else
	    {
	      sinix = __imag__ x;
	      cosix = 1.0;
	    }

	  __real__ retval = __copysignl (HUGE_VALL, cosix);
	  __imag__ retval = __copysignl (HUGE_VALL, sinix);

	  if (negate)
	    __real__ retval = -__real__ retval;
	}
      else if (icls == FP_ZERO)
	{
	  /* Imaginary part is 0.0.  */
	  __real__ retval = negate ? -HUGE_VALL : HUGE_VALL;
	  __imag__ retval = __imag__ x;
	}
      else
	{
	  /* The addition raises the invalid exception.  */
	  __real__ retval = HUGE_VALL;
	  __imag__ retval = __nanl ("") + __nanl ("");

#ifdef FE_INVALID
	  if (icls == FP_INFINITE)
	    feraiseexcept (FE_INVALID);
#endif
	}
    }
  else
    {
      __real__ retval = __nanl ("");
      __imag__ retval = __imag__ x == 0.0 ? __imag__ x : __nanl ("");
    }

  return retval;
}
weak_alias (__csinhl, csinhl)
