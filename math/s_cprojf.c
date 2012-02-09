/* Compute projection of complex float value to Riemann sphere.
   Copyright (C) 1997, 1999, 2010 Free Software Foundation, Inc.
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


__complex__ float
__cprojf (__complex__ float x)
{
  if (isnan (__real__ x) && isnan (__imag__ x))
    return x;
  else if (!isfinite (__real__ x) || !isfinite (__imag__ x))
    {
      __complex__ float res;

      __real__ res = INFINITY;
      __imag__ res = __copysignf (0.0, __imag__ x);

      return res;
    }

  return x;
}
#ifndef __cprojf
weak_alias (__cprojf, cprojf)
#endif
