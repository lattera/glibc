/* Total order operation on absolute values.  ldbl-128 version.
   Copyright (C) 2016-2018 Free Software Foundation, Inc.
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
#include <libm-alias-ldouble.h>
#include <nan-high-order-bit.h>
#include <stdint.h>

int
__totalordermagl (_Float128 x, _Float128 y)
{
  uint64_t hx, hy;
  uint64_t lx, ly;
  GET_LDOUBLE_WORDS64 (hx, lx, x);
  GET_LDOUBLE_WORDS64 (hy, ly, y);
  hx &= 0x7fffffffffffffffULL;
  hy &= 0x7fffffffffffffffULL;
#if HIGH_ORDER_BIT_IS_SET_FOR_SNAN
  /* For the preferred quiet NaN convention, this operation is a
     comparison of the representations of the absolute values of the
     arguments.  If both arguments are NaNs, invert the
     quiet/signaling bit so comparing that way works.  */
  if ((hx > 0x7fff000000000000ULL || (hx == 0x7fff000000000000ULL
				      && lx != 0))
      && (hy > 0x7fff000000000000ULL || (hy == 0x7fff000000000000ULL
					 && ly != 0)))
    {
      hx ^= 0x0000800000000000ULL;
      hy ^= 0x0000800000000000ULL;
    }
#endif
  return hx < hy || (hx == hy && lx <= ly);
}
libm_alias_ldouble (__totalordermag, totalordermag)
