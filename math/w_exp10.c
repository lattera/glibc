/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gmail.com>, 2011.

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


/*
 * wrapper exp10(x)
 */

#include <math.h>
#include <math_private.h>

double
__exp10 (double x)
{
  double z = __ieee754_exp10 (x);
  if (__builtin_expect (!__finite (z), 0)
      && __finite (x) && _LIB_VERSION != _IEEE_)
    /* exp10 overflow (46) if x > 0, underflow (47) if x < 0.  */
    return __kernel_standard (x, x, 46 + !!__signbit (x));

  return z;
}
weak_alias (__exp10, exp10)
strong_alias (__exp10, __pow10)
weak_alias (__pow10, pow10)
#ifdef NO_LONG_DOUBLE
strong_alias (__exp10, __exp10l)
weak_alias (__exp10, exp10l)
strong_alias (__exp10l, __pow10l)
weak_alias (__pow10l, pow10l)
#endif
