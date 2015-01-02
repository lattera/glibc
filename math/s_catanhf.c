/* Return arc hyperbole tangent for float value.
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
#include <math.h>
#include <math_private.h>
#include <float.h>

__complex__ float
__catanhf (__complex__ float x)
{
  __complex__ float res;
  int rcls = fpclassify (__real__ x);
  int icls = fpclassify (__imag__ x);

  if (__glibc_unlikely (rcls <= FP_INFINITE || icls <= FP_INFINITE))
    {
      if (icls == FP_INFINITE)
	{
	  __real__ res = __copysignf (0.0, __real__ x);
	  __imag__ res = __copysignf (M_PI_2, __imag__ x);
	}
      else if (rcls == FP_INFINITE || rcls == FP_ZERO)
	{
	  __real__ res = __copysignf (0.0, __real__ x);
	  if (icls >= FP_ZERO)
	    __imag__ res = __copysignf (M_PI_2, __imag__ x);
	  else
	    __imag__ res = __nanf ("");
	}
      else
	{
	  __real__ res = __nanf ("");
	  __imag__ res = __nanf ("");
	}
    }
  else if (__glibc_unlikely (rcls == FP_ZERO && icls == FP_ZERO))
    {
      res = x;
    }
  else
    {
      if (fabsf (__real__ x) >= 16.0f / FLT_EPSILON
	  || fabsf (__imag__ x) >= 16.0f / FLT_EPSILON)
	{
	  __imag__ res = __copysignf ((float) M_PI_2, __imag__ x);
	  if (fabsf (__imag__ x) <= 1.0f)
	    __real__ res = 1.0f / __real__ x;
	  else if (fabsf (__real__ x) <= 1.0f)
	    __real__ res = __real__ x / __imag__ x / __imag__ x;
	  else
	    {
	      float h = __ieee754_hypotf (__real__ x / 2.0f,
					  __imag__ x / 2.0f);
	      __real__ res = __real__ x / h / h / 4.0f;
	    }
	}
      else
	{
	  if (fabsf (__real__ x) == 1.0f
	      && fabsf (__imag__ x) < FLT_EPSILON * FLT_EPSILON)
	    __real__ res = (__copysignf (0.5f, __real__ x)
			    * ((float) M_LN2
			       - __ieee754_logf (fabsf (__imag__ x))));
	  else
	    {
	      float i2 = 0.0f;
	      if (fabsf (__imag__ x) >= FLT_EPSILON * FLT_EPSILON)
		i2 = __imag__ x * __imag__ x;

	      float num = 1.0f + __real__ x;
	      num = i2 + num * num;

	      float den = 1.0f - __real__ x;
	      den = i2 + den * den;

	      float f = num / den;
	      if (f < 0.5f)
		__real__ res = 0.25f * __ieee754_logf (f);
	      else
		{
		  num = 4.0f * __real__ x;
		  __real__ res = 0.25f * __log1pf (num / den);
		}
	    }

	  float absx, absy, den;

	  absx = fabsf (__real__ x);
	  absy = fabsf (__imag__ x);
	  if (absx < absy)
	    {
	      float t = absx;
	      absx = absy;
	      absy = t;
	    }

	  if (absy < FLT_EPSILON / 2.0f)
	    {
	      den = (1.0f - absx) * (1.0f + absx);
	      if (den == -0.0f)
		den = 0.0f;
	    }
	  else if (absx >= 1.0f)
	    den = (1.0f - absx) * (1.0f + absx) - absy * absy;
	  else if (absx >= 0.75f || absy >= 0.5f)
	    den = -__x2y2m1f (absx, absy);
	  else
	    den = (1.0f - absx) * (1.0f + absx) - absy * absy;

	  __imag__ res = 0.5f * __ieee754_atan2f (2.0f * __imag__ x, den);
	}

      if (fabsf (__real__ res) < FLT_MIN)
	{
	  volatile float force_underflow = __real__ res * __real__ res;
	  (void) force_underflow;
	}
      if (fabsf (__imag__ res) < FLT_MIN)
	{
	  volatile float force_underflow = __imag__ res * __imag__ res;
	  (void) force_underflow;
	}
    }

  return res;
}
#ifndef __catanhf
weak_alias (__catanhf, catanhf)
#endif
