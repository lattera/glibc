/* Complex hyperbole tangent for float.
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

__complex__ float
__ctanhf (__complex__ float x)
{
  __complex__ float res;

  if (__builtin_expect (!isfinite (__real__ x) || !isfinite (__imag__ x), 0))
    {
      if (__isinf_nsf (__real__ x))
	{
	  __real__ res = __copysignf (1.0, __real__ x);
	  __imag__ res = __copysignf (0.0, __imag__ x);
	}
      else if (__imag__ x == 0.0)
	{
	  res = x;
	}
      else
	{
	  __real__ res = __nanf ("");
	  __imag__ res = __nanf ("");

	  if (__isinf_nsf (__imag__ x))
	    feraiseexcept (FE_INVALID);
	}
    }
  else
    {
      float sinix, cosix;
      float den;
      const int t = (int) ((FLT_MAX_EXP - 1) * M_LN2 / 2);

      /* tanh(x+iy) = (sinh(2x) + i*sin(2y))/(cosh(2x) + cos(2y))
	 = (sinh(x)*cosh(x) + i*sin(y)*cos(y))/(sinh(x)^2 + cos(y)^2).  */

      if (__builtin_expect (fpclassify(__imag__ x) != FP_SUBNORMAL, 1))
	{
	  __sincosf (__imag__ x, &sinix, &cosix);
	}
      else
	{
	  sinix = __imag__ x;
	  cosix = 1.0f;
	}

      if (fabsf (__real__ x) > t)
	{
	  /* Avoid intermediate overflow when the imaginary part of
	     the result may be subnormal.  Ignoring negligible terms,
	     the real part is +/- 1, the imaginary part is
	     sin(y)*cos(y)/sinh(x)^2 = 4*sin(y)*cos(y)/exp(2x).  */
	  float exp_2t = __ieee754_expf (2 * t);

	  __real__ res = __copysignf (1.0, __real__ x);
	  __imag__ res = 4 * sinix * cosix;
	  __real__ x = fabsf (__real__ x);
	  __real__ x -= t;
	  __imag__ res /= exp_2t;
	  if (__real__ x > t)
	    {
	      /* Underflow (original real part of x has absolute value
		 > 2t).  */
	      __imag__ res /= exp_2t;
	    }
	  else
	    __imag__ res /= __ieee754_expf (2 * __real__ x);
	}
      else
	{
	  float sinhrx, coshrx;
	  if (fabsf (__real__ x) > FLT_MIN)
	    {
	      sinhrx = __ieee754_sinhf (__real__ x);
	      coshrx = __ieee754_coshf (__real__ x);
	    }
	  else
	    {
	      sinhrx = __real__ x;
	      coshrx = 1.0f;
	    }

	  if (fabsf (sinhrx) > fabsf (cosix) * FLT_EPSILON)
	    den = sinhrx * sinhrx + cosix * cosix;
	  else
	    den = cosix * cosix;
	  __real__ res = sinhrx * coshrx / den;
	  __imag__ res = sinix * cosix / den;
	}
    }

  return res;
}
#ifndef __ctanhf
weak_alias (__ctanhf, ctanhf)
#endif
