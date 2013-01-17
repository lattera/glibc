/* Return arc hyperbole sine for long double value, with the imaginary
   part of the result possibly adjusted for use in computing other
   functions.
   Copyright (C) 1997-2013 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

/* Return the complex inverse hyperbolic sine of finite nonzero Z,
   with the imaginary part of the result subtracted from pi/2 if ADJ
   is nonzero.  */

__complex__ long double
__kernel_casinhl (__complex__ long double x, int adj)
{
  __complex__ long double res;
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

      if (adj)
	{
	  long double t = __real__ y;
	  __real__ y = __copysignl (__imag__ y, __imag__ x);
	  __imag__ y = t;
	}

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

      if (adj)
	{
	  long double t = __real__ y;
	  __real__ y = __copysignl (__imag__ y, __imag__ x);
	  __imag__ y = t;
	}

      res = __clogl (y);
    }

  /* Give results the correct sign for the original argument.  */
  __real__ res = __copysignl (__real__ res, __real__ x);
  __imag__ res = __copysignl (__imag__ res, (adj ? 1.0L : __imag__ x));

  return res;
}
