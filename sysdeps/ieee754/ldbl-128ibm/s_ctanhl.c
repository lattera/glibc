/* Complex hyperbole tangent for long double.  IBM extended format version.
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
#include <float.h>
#include <math.h>
#include <math_ldbl_opt.h>

#include <math_private.h>


__complex__ long double
__ctanhl (__complex__ long double x)
{
  __complex__ long double res;

  if (!isfinite (__real__ x) || !isfinite (__imag__ x))
    {
      if (__isinfl (__real__ x))
	{
	  __real__ res = __copysignl (1.0, __real__ x);
	  __imag__ res = __copysignl (0.0, __imag__ x);
	}
      else if (__imag__ x == 0.0)
	{
	  res = x;
	}
      else
	{
	  __real__ res = __nanl ("");
	  __imag__ res = __nanl ("");

#ifdef FE_INVALID
	  if (__isinfl (__imag__ x))
	    feraiseexcept (FE_INVALID);
#endif
	}
    }
  else
    {
      long double sinix, cosix;
      long double den;
      const int t = (int) ((LDBL_MAX_EXP - 1) * M_LN2l / 2);

      /* tanh(x+iy) = (sinh(2x) + i*sin(2y))/(cosh(2x) + cos(2y))
        = (sinh(x)*cosh(x) + i*sin(y)*cos(y))/(sinh(x)^2 + cos(y)^2).  */

      __sincosl (__imag__ x, &sinix, &cosix);

      if (fabsl (__real__ x) > t)
	{
	  /* Avoid intermediate overflow when the imaginary part of
	     the result may be subnormal.  Ignoring negligible terms,
	     the real part is +/- 1, the imaginary part is
	     sin(y)*cos(y)/sinh(x)^2 = 4*sin(y)*cos(y)/exp(2x).  */
	  long double exp_2t = __ieee754_expl (2 * t);
	  __real__ res = __copysignl (1.0, __real__ x);
	  __imag__ res = 4 * sinix * cosix;
	  __real__ x = fabsl (__real__ x);
	  __real__ x -= t;
	  __imag__ res /= exp_2t;
	  if (__real__ x > t)
	    {
	      /* Underflow (original real part of x has absolute value
		 > 2t).  */
	      __imag__ res /= exp_2t;
	    }
	  else
	    __imag__ res /= __ieee754_expl (2 * __real__ x);
	}
      else
	{
	  long double sinhrx = __ieee754_sinhl (__real__ x);
	  long double coshrx = __ieee754_coshl (__real__ x);

	  den = sinhrx * sinhrx + cosix * cosix;
	  __real__ res = sinhrx * coshrx / den;
	  __imag__ res = sinix * cosix / den;
	}
      /* __gcc_qmul does not respect -0.0 so we need the following fixup.  */
      if ((__real__ res == 0.0) && (__real__ x == 0.0))
        __real__ res = __real__ x;

      if ((__real__ res == 0.0) && (__imag__ x == 0.0))
        __imag__ res = __imag__ x;
    }

  return res;
}
long_double_symbol (libm, __ctanhl, ctanhl);
