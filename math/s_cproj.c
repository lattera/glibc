/* Compute projection of complex double value to Riemann sphere.
   Copyright (C) 1997-2015 Free Software Foundation, Inc.
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


__complex__ double
__cproj (__complex__ double x)
{
  if (__isinf_ns (__real__ x) || __isinf_ns (__imag__ x))
    {
      __complex__ double res;

      __real__ res = INFINITY;
      __imag__ res = __copysign (0.0, __imag__ x);

      return res;
    }

  return x;
}
weak_alias (__cproj, cproj)
#ifdef NO_LONG_DOUBLE
strong_alias (__cproj, __cprojl)
weak_alias (__cproj, cprojl)
#endif
