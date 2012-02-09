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

#include <fenv.h>
#include <math.h>
#include <math_private.h>


/* wrapper acosl */
long double
__acosl (long double x)
{
  if (__builtin_expect (fabsl (x) > 1.0L, 0) && _LIB_VERSION != _IEEE_)
    {
      /* acos(|x|>1) */
      feraiseexcept (FE_INVALID);
      return __kernel_standard (x, x, 201);
    }

  return __ieee754_acosl (x);
}
weak_alias (__acosl, acosl)
