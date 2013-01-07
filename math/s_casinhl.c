/* Return arc hyperbole sine for long double value.
   Copyright (C) 1997-2013 Free Software Foundation, Inc.
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

/* To avoid spurious overflows, use this definition to treat IBM long
   double as approximating an IEEE-style format.  */
#if LDBL_MANT_DIG == 106
# undef LDBL_EPSILON
# define LDBL_EPSILON 0x1p-106L
#endif

__complex__ long double
__casinhl (__complex__ long double x)
{
  __complex__ long double res;
  int rcls = fpclassify (__real__ x);
  int icls = fpclassify (__imag__ x);

  if (rcls <= FP_INFINITE || icls <= FP_INFINITE)
    {
      if (icls == FP_INFINITE)
	{
	  __real__ res = __copysignl (HUGE_VALL, __real__ x);

	  if (rcls == FP_NAN)
	    __imag__ res = __nanl ("");
	  else
	    __imag__ res = __copysignl (rcls >= FP_ZERO ? M_PI_2l : M_PI_4l,
					__imag__ x);
	}
      else if (rcls <= FP_INFINITE)
	{
	  __real__ res = __real__ x;
	  if ((rcls == FP_INFINITE && icls >= FP_ZERO)
	      || (rcls == FP_NAN && icls == FP_ZERO))
	    __imag__ res = __copysignl (0.0, __imag__ x);
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
      res = x;
    }
  else
    {
      long double rx, ix;
      __complex__ long double y;

      /* Avoid cancellation by reducing to the first quadrant.  */
      rx = fabsl (__real__ x);
      ix = fabsl (__imag__ x);

      if (rx >= 1.0L / LDBL_EPSILON || ix >= 1.0L / LDBL_EPSILON)
	{
	  /* For large x in the first quadrant, x + csqrt (1 + x * x)
	     is sufficiently close to 2 * x to make no significant
	     difference to the result; avoid possible overflow from
	     the squaring and addition.  */
	  __real__ y = rx;
	  __imag__ y = ix;
	  res = __clogl (y);
	  __real__ res += M_LN2l;
	}
      else
	{
	  __real__ y = (rx - ix) * (rx + ix) + 1.0;
	  __imag__ y = 2.0 * rx * ix;

	  y = __csqrtl (y);

	  __real__ y += rx;
	  __imag__ y += ix;

	  res = __clogl (y);
	}

      /* Give results the correct sign for the original argument.  */
      __real__ res = __copysignl (__real__ res, __real__ x);
      __imag__ res = __copysignl (__imag__ res, __imag__ x);
    }

  return res;
}
weak_alias (__casinhl, casinhl)
