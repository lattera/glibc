/* Compute radix independent exponent.
   Copyright (C) 2011 Free Software Foundation, Inc.
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

#include "math_private.h"


double
__logb (double x)
{
  int64_t ix;

  EXTRACT_WORDS64 (ix, x);
  ix &= UINT64_C(0x7fffffffffffffff);
  if (ix == 0)
    return -1.0 / fabs (x);
  unsigned int ex = ix >> 52;
  if (ex == 0x7ff)
    return x * x;
  return ex == 0 ? -1022.0 : (double) (ex - 1023);
}
weak_alias (__logb, logb)
#ifdef NO_LONG_DOUBLE
strong_alias (__logb, __logbl)
weak_alias (__logb, logbl)
#endif
