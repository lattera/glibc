/* Total order operation on absolute values.  dbl-64 version.
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
  uint32_t hx, hy;
  uint32_t lx, ly;
  EXTRACT_WORDS (hx, lx, x);
  EXTRACT_WORDS (hy, ly, y);
  hx &= 0x7fffffff;
  hy &= 0x7fffffff;
#if HIGH_ORDER_BIT_IS_SET_FOR_SNAN
  /* For the preferred quiet NaN convention, this operation is a
     comparison of the representations of the absolute values of the
     arguments.  If both arguments are NaNs, invert the
     quiet/signaling bit so comparing that way works.  */
  if ((hx > 0x7ff00000 || (hx == 0x7ff00000 && lx != 0))
      && (hy > 0x7ff00000 || (hy == 0x7ff00000 && ly != 0)))
    {
      hx ^= 0x00080000;
      hy ^= 0x00080000;
    }
#endif
  return hx < hy || (hx == hy && lx <= ly);
}
#ifdef NO_LONG_DOUBLE
weak_alias (totalordermag, totalordermagl)
#endif
