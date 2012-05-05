/* Copyright (C) 2011, 2012 Free Software Foundation, Inc.
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

#include <math.h>
#include <math_private.h>

/* wrapper exp */
double
__exp (double x)
{
  double z = __ieee754_exp (x);
  if (__builtin_expect (!__finite (z) || z == 0, 0)
      && __finite (x) && _LIB_VERSION != _IEEE_)
    return __kernel_standard (x, x, 6 + !!__signbit (x));

  return z;
}
hidden_def (__exp)
weak_alias (__exp, exp)
#ifdef NO_LONG_DOUBLE
strong_alias (__exp, __expl)
weak_alias (__exp, expl)
#endif
