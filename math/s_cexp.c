/* Return value of complex exponential function for double complex value.
   Copyright (C) 1997-2015 Free Software Foundation, Inc.
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

__complex__ double
__cexp (__complex__ double x)
{
  __complex__ double retval;
  int rcls = fpclassify (__real__ x);
  int icls = fpclassify (__imag__ x);

  if (__glibc_likely (rcls >= FP_ZERO))
    {
      /* Real part is finite.  */
      if (__glibc_likely (icls >= FP_ZERO))
	{
	  /* Imaginary part is finite.  */
	  const int t = (int) ((DBL_MAX_EXP - 1) * M_LN2);
	  double sinix, cosix;

	  if (__glibc_likely (icls != FP_SUBNORMAL))
	    {
	      __sincos (__imag__ x, &sinix, &cosix);
	    }
	  else
	    {
	      sinix = __imag__ x;
	      cosix = 1.0;
	    }

	  if (__real__ x > t)
	    {
	      double exp_t = __ieee754_exp (t);
	      __real__ x -= t;
	      sinix *= exp_t;
	      cosix *= exp_t;
	      if (__real__ x > t)
		{
		  __real__ x -= t;
		  sinix *= exp_t;
		  cosix *= exp_t;
		}
	    }
	  if (__real__ x > t)
	    {
	      /* Overflow (original real part of x > 3t).  */
	      __real__ retval = DBL_MAX * cosix;
	      __imag__ retval = DBL_MAX * sinix;
	    }
	  else
	    {
	      double exp_val = __ieee754_exp (__real__ x);
	      __real__ retval = exp_val * cosix;
	      __imag__ retval = exp_val * sinix;
	    }
	  if (fabs (__real__ retval) < DBL_MIN)
	    {
	      volatile double force_underflow
		= __real__ retval * __real__ retval;
	      (void) force_underflow;
	    }
	  if (fabs (__imag__ retval) < DBL_MIN)
	    {
	      volatile double force_underflow
		= __imag__ retval * __imag__ retval;
	      (void) force_underflow;
	    }
	}
      else
	{
	  /* If the imaginary part is +-inf or NaN and the real part
	     is not +-inf the result is NaN + iNaN.  */
	  __real__ retval = __nan ("");
	  __imag__ retval = __nan ("");

	  feraiseexcept (FE_INVALID);
	}
    }
  else if (__glibc_likely (rcls == FP_INFINITE))
    {
      /* Real part is infinite.  */
      if (__glibc_likely (icls >= FP_ZERO))
	{
	  /* Imaginary part is finite.  */
	  double value = signbit (__real__ x) ? 0.0 : HUGE_VAL;

	  if (icls == FP_ZERO)
	    {
	      /* Imaginary part is 0.0.  */
	      __real__ retval = value;
	      __imag__ retval = __imag__ x;
	    }
	  else
	    {
	      double sinix, cosix;

	      if (__glibc_likely (icls != FP_SUBNORMAL))
		{
		  __sincos (__imag__ x, &sinix, &cosix);
		}
	      else
		{
		  sinix = __imag__ x;
		  cosix = 1.0;
		}

	      __real__ retval = __copysign (value, cosix);
	      __imag__ retval = __copysign (value, sinix);
	    }
	}
      else if (signbit (__real__ x) == 0)
	{
	  __real__ retval = HUGE_VAL;
	  __imag__ retval = __nan ("");

	  if (icls == FP_INFINITE)
	    feraiseexcept (FE_INVALID);
	}
      else
	{
	  __real__ retval = 0.0;
	  __imag__ retval = __copysign (0.0, __imag__ x);
	}
    }
  else
    {
      /* If the real part is NaN the result is NaN + iNaN unless the
	 imaginary part is zero.  */
      __real__ retval = __nan ("");
      if (icls == FP_ZERO)
	__imag__ retval = __imag__ x;
      else
	{
	  __imag__ retval = __nan ("");

	  if (rcls != FP_NAN || icls != FP_NAN)
	    feraiseexcept (FE_INVALID);
	}
    }

  return retval;
}
weak_alias (__cexp, cexp)
#ifdef NO_LONG_DOUBLE
strong_alias (__cexp, __cexpl)
weak_alias (__cexp, cexpl)
#endif
