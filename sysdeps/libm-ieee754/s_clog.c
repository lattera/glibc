/* Compute complex natural logarithm.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <complex.h>
#include <math.h>

#include "math_private.h"


__complex__ double
__clog (__complex__ double x)
{
  __complex__ double result;

  if (__real__ x == 0.0 && __imag__ x == 0.0)
    {
      __imag__ result = signbit (__real__ x) ? M_PI : 0.0;
      if (signbit (__imag__ x))
	__imag__ result = __copysign (__imag__ result, -1.0);
      /* Yes, the following line raises an exception.  */
      __real__ result = -1.0 / fabs (__real__ x);
    }
  else if (!__isnan (__real__ x) && !__isnan (__imag__ x))
    {
      __real__ result = __ieee754_log (__ieee754_hypot (__real__ x,
							__imag__ x));
      __imag__ result = __ieee754_atan2 (__imag__ x, __real__ x);
    }
  else
    {
      __imag__ result = __nan ("");
      if (__isinf (__real__ x) || __isinf (__imag__ x))
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
