/* Total order operation on absolute values.  dbl-64/wordsize-64 version.
   Copyright (C) 2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
#include <nan-high-order-bit.h>
#include <stdint.h>

int
totalordermag (double x, double y)
{
  uint64_t ix, iy;
  EXTRACT_WORDS64 (ix, x);
  EXTRACT_WORDS64 (iy, y);
  ix &= 0x7fffffffffffffffULL;
  iy &= 0x7fffffffffffffffULL;
#if HIGH_ORDER_BIT_IS_SET_FOR_SNAN
  /* For the preferred quiet NaN convention, this operation is a
     comparison of the representations of the absolute values of the
     arguments.  If both arguments are NaNs, invert the
     quiet/signaling bit so comparing that way works.  */
  if (ix > 0x7ff0000000000000ULL && iy > 0x7ff0000000000000ULL)
    {
      ix ^= 0x0008000000000000ULL;
      iy ^= 0x0008000000000000ULL;
    }
#endif
  return ix <= iy;
}
#ifdef NO_LONG_DOUBLE
weak_alias (totalordermag, totalordermagl)
#endif
