/* Return arc hyperbole cosine for double value.
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


__complex__ double
__cacosh (__complex__ double x)
{
  __complex__ double res;
  int rcls = fpclassify (__real__ x);
  int icls = fpclassify (__imag__ x);

  if (rcls <= FP_INFINITE || icls <= FP_INFINITE)
    {
      if (icls == FP_INFINITE)
	{
	  __real__ res = HUGE_VAL;

	  if (rcls == FP_NAN)
	    __imag__ res = __nan ("");
	  else
	    __imag__ res = __copysign ((rcls == FP_INFINITE
					? (__real__ x < 0.0
					   ? M_PI - M_PI_4 : M_PI_4)
					: M_PI_2), __imag__ x);
	}
      else if (rcls == FP_INFINITE)
	{
	  __real__ res = HUGE_VAL;

	  if (icls >= FP_ZERO)
	    __imag__ res = __copysign (signbit (__real__ x) ? M_PI : 0.0,
				       __imag__ x);
	  else
	    __imag__ res = __nan ("");
	}
      else
	{
	  __real__ res = __nan ("");
	  __imag__ res = __nan ("");
	}
    }
  else if (rcls == FP_ZERO && icls == FP_ZERO)
    {
      __real__ res = 0.0;
      __imag__ res = __copysign (M_PI_2, __imag__ x);
    }
  /* The factor 16 is just a guess.  */
  else if (16.0 * fabs (__imag__ x) < fabs (__real__ x))
    {
      /* Kahan's formula which avoid cancellation through subtraction in
	 some cases.  */
      res = 2.0 * __clog (__csqrt ((x + 1.0) / 2.0) + __csqrt ((x - 1.0) / 2.0));
      if (signbit (__real__ res))
	__real__ res = 0.0;
    }
  else
    {
      __complex__ double y;

      __real__ y = (__real__ x - __imag__ x) * (__real__ x + __imag__ x) - 1.0;
      __imag__ y = 2.0 * __real__ x * __imag__ x;

      y = __csqrt (y);

      if (signbit (__real__ x))
	y = -y;

      __real__ y += __real__ x;
      __imag__ y += __imag__ x;

      res = __clog (y);
    }

  return res;
}
weak_alias (__cacosh, cacosh)
#ifdef NO_LONG_DOUBLE
strong_alias (__cacosh, __cacoshl)
weak_alias (__cacosh, cacoshl)
#endif
