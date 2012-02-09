/* Return arc hyperbole cosine for long double value.
   Copyright (C) 1997, 1998, 2006, 2011 Free Software Foundation, Inc.
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
__cacoshl (__complex__ long double x)
{
  __complex__ long double res;
  int rcls = fpclassify (__real__ x);
  int icls = fpclassify (__imag__ x);

  if (rcls <= FP_INFINITE || icls <= FP_INFINITE)
    {
      if (icls == FP_INFINITE)
	{
	  __real__ res = HUGE_VALL;

	  if (rcls == FP_NAN)
	    __imag__ res = __nanl ("");
	  else
	    __imag__ res = __copysignl ((rcls == FP_INFINITE
					 ? (__real__ x < 0.0
					    ? M_PIl - M_PI_4l : M_PI_4l)
					 : M_PI_2l), __imag__ x);
	}
      else if (rcls == FP_INFINITE)
	{
	  __real__ res = HUGE_VALL;

	  if (icls >= FP_ZERO)
	    __imag__ res = __copysignl (signbit (__real__ x) ? M_PIl : 0.0,
					__imag__ x);
	  else
	    __imag__ res = __nanl ("");
	}
      else
	{
	  __real__ res = __nanl ("");
	  __imag__ res = __nanl ("");
	}
    }
  else if (rcls == FP_ZERO && icls == FP_ZERO)
    {
      __real__ res = 0.0;
      __imag__ res = __copysignl (M_PI_2l, __imag__ x);
    }
  /* The factor 16 is just a guess.  */
  else if (16.0L * fabsl (__imag__ x) < fabsl (__real__ x))
    /* Kahan's formula which avoid cancellation through subtraction in
       some cases.  */
    res = 2.0L * __clogl (__csqrtl ((x + 1.0L) / 2.0L)
			  + __csqrtl ((x - 1.0L) / 2.0L));
  else
    {
      __complex__ long double y;

      __real__ y = (__real__ x - __imag__ x) * (__real__ x + __imag__ x) - 1.0;
      __imag__ y = 2.0 * __real__ x * __imag__ x;

      y = __csqrtl (y);

      if (signbit (__real__ x))
	y = -y;

      __real__ y += __real__ x;
      __imag__ y += __imag__ x;

      res = __clogl (y);
    }

  return res;
}
weak_alias (__cacoshl, cacoshl)
