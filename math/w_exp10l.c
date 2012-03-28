/* Copyright (C) 2011-2012 Free Software Foundation, Inc.
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
 * wrapper exp10l(x)
 */

#include <math.h>
#include <math_private.h>

long double
__exp10l (long double x)
{
  long double z = __ieee754_exp10l (x);
  if (__builtin_expect (!__finitel (z), 0)
      && __finitel (x) && _LIB_VERSION != _IEEE_)
    /* exp10l overflow (246) if x > 0, underflow (247) if x < 0.  */
    return __kernel_standard_l (x, x, 246 + !!__signbitl (x));

  return z;
}
weak_alias (__exp10l, exp10l)
strong_alias (__exp10l, __pow10l)
weak_alias (__pow10l, pow10l)
