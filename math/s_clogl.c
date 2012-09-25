/* Compute complex natural logarithm.
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

/* To avoid spurious underflows, use this definition to treat IBM long
   double as approximating an IEEE-style format.  */
#if LDBL_MANT_DIG == 106
# undef LDBL_EPSILON
# define LDBL_EPSILON 0x1p-106L
#endif

__complex__ long double
__clogl (__complex__ long double x)
{
  __complex__ long double result;
  int rcls = fpclassify (__real__ x);
  int icls = fpclassify (__imag__ x);

  if (__builtin_expect (rcls == FP_ZERO && icls == FP_ZERO, 0))
    {
      /* Real and imaginary part are 0.0.  */
      __imag__ result = signbit (__real__ x) ? M_PIl : 0.0;
      __imag__ result = __copysignl (__imag__ result, __imag__ x);
      /* Yes, the following line raises an exception.  */
      __real__ result = -1.0 / fabsl (__real__ x);
    }
  else if (__builtin_expect (rcls != FP_NAN && icls != FP_NAN, 1))
    {
      /* Neither real nor imaginary part is NaN.  */
      long double absx = fabsl (__real__ x), absy = fabsl (__imag__ x);
      int scale = 0;

      if (absx < absy)
	{
	  long double t = absx;
	  absx = absy;
	  absy = t;
	}

      if (absx > LDBL_MAX / 2.0L)
	{
	  scale = -1;
	  absx = __scalbnl (absx, scale);
	  absy = (absy >= LDBL_MIN * 2.0L ? __scalbnl (absy, scale) : 0.0L);
	}
      else if (absx < LDBL_MIN && absy < LDBL_MIN)
	{
	  scale = LDBL_MANT_DIG;
	  absx = __scalbnl (absx, scale);
	  absy = __scalbnl (absy, scale);
	}

      if (absx == 1.0L && scale == 0)
	{
	  long double absy2 = absy * absy;
	  if (absy2 <= LDBL_MIN * 2.0L)
	    __real__ result = absy2 / 2.0L - absy2 * absy2 / 4.0L;
	  else
	    __real__ result = __log1pl (absy2) / 2.0L;
	}
      else if (absx > 1.0L && absx < 2.0L && absy < 1.0L && scale == 0)
	{
	  long double d2m1 = (absx - 1.0L) * (absx + 1.0L);
	  if (absy >= LDBL_EPSILON)
	    d2m1 += absy * absy;
	  __real__ result = __log1pl (d2m1) / 2.0L;
	}
      else if (absx < 1.0L
	       && absx >= 0.75L
	       && absy < LDBL_EPSILON / 2.0L
	       && scale == 0)
	{
	  long double d2m1 = (absx - 1.0L) * (absx + 1.0L);
	  __real__ result = __log1pl (d2m1) / 2.0L;
	}
      else if (absx < 1.0L && (absx >= 0.75L || absy >= 0.5L) && scale == 0)
	{
	  long double d2m1 = __x2y2m1l (absx, absy);
	  __real__ result = __log1pl (d2m1) / 2.0L;
	}
      else
	{
	  long double d = __ieee754_hypotl (absx, absy);
	  __real__ result = __ieee754_logl (d) - scale * M_LN2l;
	}

      __imag__ result = __ieee754_atan2l (__imag__ x, __real__ x);
    }
  else
    {
      __imag__ result = __nanl ("");
      if (rcls == FP_INFINITE || icls == FP_INFINITE)
	/* Real or imaginary part is infinite.  */
	__real__ result = HUGE_VALL;
      else
	__real__ result = __nanl ("");
    }

  return result;
}
weak_alias (__clogl, clogl)
