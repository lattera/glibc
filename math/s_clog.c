/* Compute complex natural logarithm.
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

__complex__ double
__clog (__complex__ double x)
{
  __complex__ double result;
  int rcls = fpclassify (__real__ x);
  int icls = fpclassify (__imag__ x);

  if (__glibc_unlikely (rcls == FP_ZERO && icls == FP_ZERO))
    {
      /* Real and imaginary part are 0.0.  */
      __imag__ result = signbit (__real__ x) ? M_PI : 0.0;
      __imag__ result = __copysign (__imag__ result, __imag__ x);
      /* Yes, the following line raises an exception.  */
      __real__ result = -1.0 / fabs (__real__ x);
    }
  else if (__glibc_likely (rcls != FP_NAN && icls != FP_NAN))
    {
      /* Neither real nor imaginary part is NaN.  */
      double absx = fabs (__real__ x), absy = fabs (__imag__ x);
      int scale = 0;

      if (absx < absy)
	{
	  double t = absx;
	  absx = absy;
	  absy = t;
	}

      if (absx > DBL_MAX / 2.0)
	{
	  scale = -1;
	  absx = __scalbn (absx, scale);
	  absy = (absy >= DBL_MIN * 2.0 ? __scalbn (absy, scale) : 0.0);
	}
      else if (absx < DBL_MIN && absy < DBL_MIN)
	{
	  scale = DBL_MANT_DIG;
	  absx = __scalbn (absx, scale);
	  absy = __scalbn (absy, scale);
	}

      if (absx == 1.0 && scale == 0)
	{
	  double absy2 = absy * absy;
	  if (absy2 <= DBL_MIN * 2.0)
	    {
	      double force_underflow = absy2 * absy2;
	      __real__ result = absy2 / 2.0;
	      math_force_eval (force_underflow);
	    }
	  else
	    __real__ result = __log1p (absy2) / 2.0;
	}
      else if (absx > 1.0 && absx < 2.0 && absy < 1.0 && scale == 0)
	{
	  double d2m1 = (absx - 1.0) * (absx + 1.0);
	  if (absy >= DBL_EPSILON)
	    d2m1 += absy * absy;
	  __real__ result = __log1p (d2m1) / 2.0;
	}
      else if (absx < 1.0
	       && absx >= 0.75
	       && absy < DBL_EPSILON / 2.0
	       && scale == 0)
	{
	  double d2m1 = (absx - 1.0) * (absx + 1.0);
	  __real__ result = __log1p (d2m1) / 2.0;
	}
      else if (absx < 1.0 && (absx >= 0.75 || absy >= 0.5) && scale == 0)
	{
	  double d2m1 = __x2y2m1 (absx, absy);
	  __real__ result = __log1p (d2m1) / 2.0;
	}
      else
	{
	  double d = __ieee754_hypot (absx, absy);
	  __real__ result = __ieee754_log (d) - scale * M_LN2;
	}

      __imag__ result = __ieee754_atan2 (__imag__ x, __real__ x);
    }
  else
    {
      __imag__ result = __nan ("");
      if (rcls == FP_INFINITE || icls == FP_INFINITE)
	/* Real or imaginary part is infinite.  */
	__real__ result = HUGE_VAL;
      else
	__real__ result = __nan ("");
    }

  return result;
}
weak_alias (__clog, clog)
#ifdef NO_LONG_DOUBLE
strong_alias (__clog, __clogl)
weak_alias (__clog, clogl)
#endif
