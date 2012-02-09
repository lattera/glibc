/* Return arc hyperbole tangent for long double value.
   Copyright (C) 1997, 1998, 2011 Free Software Foundation, Inc.
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


__complex__ long double
__catanhl (__complex__ long double x)
{
  __complex__ long double res;
  int rcls = fpclassify (__real__ x);
  int icls = fpclassify (__imag__ x);

  if (__builtin_expect (rcls <= FP_INFINITE || icls <= FP_INFINITE, 0))
    {
      if (icls == FP_INFINITE)
	{
	  __real__ res = __copysignl (0.0, __real__ x);
	  __imag__ res = __copysignl (M_PI_2l, __imag__ x);
	}
      else if (rcls == FP_INFINITE || rcls == FP_ZERO)
	{
	  __real__ res = __copysignl (0.0, __real__ x);
	  if (icls >= FP_ZERO)
	    __imag__ res = __copysignl (M_PI_2l, __imag__ x);
	  else
	    __imag__ res = __nanl ("");
	}
      else
	{
	  __real__ res = __nanl ("");
	  __imag__ res = __nanl ("");
	}
    }
  else if (__builtin_expect (rcls == FP_ZERO && icls == FP_ZERO, 0))
    {
      res = x;
    }
  else
    {
      long double i2 = __imag__ x * __imag__ x;

      long double num = 1.0 + __real__ x;
      num = i2 + num * num;

      long double den = 1.0 - __real__ x;
      den = i2 + den * den;

      __real__ res = 0.25 * (__ieee754_logl (num) - __ieee754_logl (den));

      den = 1 - __real__ x * __real__ x - i2;

      __imag__ res = 0.5 * __ieee754_atan2l (2.0 * __imag__ x, den);
    }

  return res;
}
weak_alias (__catanhl, catanhl)
