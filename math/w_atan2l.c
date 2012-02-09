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
 * wrapper atan2l(y,x)
 */

#include <math.h>
#include <math_private.h>


long double
__atan2l (long double y, long double x)
{
  if (__builtin_expect (x == 0.0L && y == 0.0L, 0) && _LIB_VERSION == _SVID_)
    return __kernel_standard (y, x, 203); /* atan2(+-0,+-0) */

  return __ieee754_atan2l (y, x);
}
weak_alias (__atan2l, atan2l)
