/* Compute complex base 10 logarithm.
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
#include <math.h>
#include <math_private.h>
#include <float.h>

/* log_10 (2).  */
#define M_LOG10_2f 0.3010299956639811952137388947244930267682f

__complex__ float
__clog10f (__complex__ float x)
{
  __complex__ float result;
  int rcls = fpclassify (__real__ x);
  int icls = fpclassify (__imag__ x);

  if (__builtin_expect (rcls == FP_ZERO && icls == FP_ZERO, 0))
    {
      /* Real and imaginary part are 0.0.  */
      __imag__ result = signbit (__real__ x) ? M_PI : 0.0;
      __imag__ result = __copysignf (__imag__ result, __imag__ x);
      /* Yes, the following line raises an exception.  */
      __real__ result = -1.0 / fabsf (__real__ x);
    }
  else if (__builtin_expect (rcls != FP_NAN && icls != FP_NAN, 1))
    {
      /* Neither real nor imaginary part is NaN.  */
      float absx = fabsf (__real__ x), absy = fabsf (__imag__ x);
      int scale = 0;

      if (absx < absy)
	{
	  float t = absx;
	  absx = absy;
	  absy = t;
	}

      if (absx > FLT_MAX / 2.0f)
	{
	  scale = -1;
	  absx = __scalbnf (absx, scale);
	  absy = (absy >= FLT_MIN * 2.0f ? __scalbnf (absy, scale) : 0.0f);
	}
      else if (absx < FLT_MIN && absy < FLT_MIN)
	{
	  scale = FLT_MANT_DIG;
	  absx = __scalbnf (absx, scale);
	  absy = __scalbnf (absy, scale);
	}

      if (absx == 1.0f && scale == 0)
	{
	  float absy2 = absy * absy;
	  if (absy2 <= FLT_MIN * 2.0f * (float) M_LN10)
	    {
#if __FLT_EVAL_METHOD__ == 0
	      __real__ result
		= (absy2 / 2.0f - absy2 * absy2 / 4.0f) * (float) M_LOG10E;
#else
	      volatile float force_underflow = absy2 * absy2 / 4.0f;
	      __real__ result
		= (absy2 / 2.0f - force_underflow) * (float) M_LOG10E;
#endif
	    }
	  else
	    __real__ result = __log1pf (absy2) * ((float) M_LOG10E / 2.0f);
	}
      else if (absx > 1.0f && absx < 2.0f && absy < 1.0f && scale == 0)
	{
	  float d2m1 = (absx - 1.0f) * (absx + 1.0f);
	  if (absy >= FLT_EPSILON)
	    d2m1 += absy * absy;
	  __real__ result = __log1pf (d2m1) * ((float) M_LOG10E / 2.0f);
	}
      else if (absx < 1.0f
	       && absx >= 0.75f
	       && absy < FLT_EPSILON / 2.0f
	       && scale == 0)
	{
	  float d2m1 = (absx - 1.0f) * (absx + 1.0f);
	  __real__ result = __log1pf (d2m1) * ((float) M_LOG10E / 2.0f);
	}
      else if (absx < 1.0f && (absx >= 0.75f || absy >= 0.5f) && scale == 0)
	{
	  float d2m1 = __x2y2m1f (absx, absy);
	  __real__ result = __log1pf (d2m1) * ((float) M_LOG10E / 2.0f);
	}
      else
	{
	  float d = __ieee754_hypotf (absx, absy);
	  __real__ result = __ieee754_log10f (d) - scale * M_LOG10_2f;
	}

      __imag__ result = M_LOG10E * __ieee754_atan2f (__imag__ x, __real__ x);
    }
  else
    {
      __imag__ result = __nanf ("");
      if (rcls == FP_INFINITE || icls == FP_INFINITE)
	/* Real or imaginary part is infinite.  */
	__real__ result = HUGE_VALF;
      else
	__real__ result = __nanf ("");
    }

  return result;
}
#ifndef __clog10f
weak_alias (__clog10f, clog10f)
#endif
