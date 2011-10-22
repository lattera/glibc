/* Complex hyperbole tangent for double.
   Copyright (C) 1997, 2005, 2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <complex.h>
#include <fenv.h>
#include <math.h>
#include <math_private.h>


__complex__ double
__ctanh (__complex__ double x)
{
  __complex__ double res;

  if (__builtin_expect (!isfinite (__real__ x) || !isfinite (__imag__ x), 0))
    {
      if (__isinf_ns (__real__ x))
	{
	  __real__ res = __copysign (1.0, __real__ x);
	  __imag__ res = __copysign (0.0, __imag__ x);
	}
      else if (__imag__ x == 0.0)
	{
	  res = x;
	}
      else
	{
	  __real__ res = __nan ("");
	  __imag__ res = __nan ("");

	  if (__isinf_ns (__imag__ x))
	    feraiseexcept (FE_INVALID);
	}
    }
  else
    {
      double sin2ix, cos2ix;
      double den;

      __sincos (2.0 * __imag__ x, &sin2ix, &cos2ix);

      den = (__ieee754_cosh (2.0 * __real__ x) + cos2ix);

      if (den == 0.0)
	{
	  __complex__ double ez = __cexp (x);
	  __complex__ double emz = __cexp (-x);

	  res = (ez - emz) / (ez + emz);
	}
      else
	{
	  __real__ res = __ieee754_sinh (2.0 * __real__ x) / den;
	  __imag__ res = sin2ix / den;
	}
    }

  return res;
}
weak_alias (__ctanh, ctanh)
#ifdef NO_LONG_DOUBLE
strong_alias (__ctanh, __ctanhl)
weak_alias (__ctanh, ctanhl)
#endif
