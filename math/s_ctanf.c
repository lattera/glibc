/* Complex tangent function for float.
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

__complex__ float
__ctanf (__complex__ float x)
{
  __complex__ float res;

  if (__glibc_unlikely (!isfinite (__real__ x) || !isfinite (__imag__ x)))
    {
      if (__isinf_nsf (__imag__ x))
	{
	  __real__ res = __copysignf (0.0, __real__ x);
	  __imag__ res = __copysignf (1.0, __imag__ x);
	}
      else if (__real__ x == 0.0)
	{
	  res = x;
	}
      else
	{
	  __real__ res = __nanf ("");
	  __imag__ res = __nanf ("");

	  if (__isinf_nsf (__real__ x))
	    feraiseexcept (FE_INVALID);
	}
    }
  else
    {
      float sinrx, cosrx;
      float den;
      const int t = (int) ((FLT_MAX_EXP - 1) * M_LN2 / 2);

      /* tan(x+iy) = (sin(2x) + i*sinh(2y))/(cos(2x) + cosh(2y))
	 = (sin(x)*cos(x) + i*sinh(y)*cosh(y)/(cos(x)^2 + sinh(y)^2). */

      if (__glibc_likely (fpclassify(__real__ x) != FP_SUBNORMAL))
	{
	  __sincosf (__real__ x, &sinrx, &cosrx);
	}
      else
	{
	  sinrx = __real__ x;
	  cosrx = 1.0f;
	}

      if (fabsf (__imag__ x) > t)
	{
	  /* Avoid intermediate overflow when the real part of the
	     result may be subnormal.  Ignoring negligible terms, the
	     imaginary part is +/- 1, the real part is
	     sin(x)*cos(x)/sinh(y)^2 = 4*sin(x)*cos(x)/exp(2y).  */
	  float exp_2t = __ieee754_expf (2 * t);

	  __imag__ res = __copysignf (1.0, __imag__ x);
	  __real__ res = 4 * sinrx * cosrx;
	  __imag__ x = fabsf (__imag__ x);
	  __imag__ x -= t;
	  __real__ res /= exp_2t;
	  if (__imag__ x > t)
	    {
	      /* Underflow (original imaginary part of x has absolute
		 value > 2t).  */
	      __real__ res /= exp_2t;
	    }
	  else
	    __real__ res /= __ieee754_expf (2 * __imag__ x);
	}
      else
	{
	  float sinhix, coshix;
	  if (fabsf (__imag__ x) > FLT_MIN)
	    {
	      sinhix = __ieee754_sinhf (__imag__ x);
	      coshix = __ieee754_coshf (__imag__ x);
	    }
	  else
	    {
	      sinhix = __imag__ x;
	      coshix = 1.0f;
	    }

	  if (fabsf (sinhix) > fabsf (cosrx) * FLT_EPSILON)
	    den = cosrx * cosrx + sinhix * sinhix;
	  else
	    den = cosrx * cosrx;
	  __real__ res = sinrx * cosrx / den;
	  __imag__ res = sinhix * coshix / den;
	}
    }

  return res;
}
#ifndef __ctanf
weak_alias (__ctanf, ctanf)
#endif
