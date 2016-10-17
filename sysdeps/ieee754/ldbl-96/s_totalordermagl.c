/* Total order operation on absolute values.  ldbl-96 version.
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

#include <float.h>
#include <math.h>
#include <math_private.h>
#include <nan-high-order-bit.h>
#include <stdint.h>

int
totalordermagl (long double x, long double y)
{
  uint16_t expx, expy;
  uint32_t hx, hy;
  uint32_t lx, ly;
  GET_LDOUBLE_WORDS (expx, hx, lx, x);
  GET_LDOUBLE_WORDS (expy, hy, ly, y);
  expx &= 0x7fff;
  expy &= 0x7fff;
  if (LDBL_MIN_EXP == -16382)
    {
      /* M68K variant: for the greatest exponent, the high mantissa
	 bit is not significant and both values of it are valid, so
	 set it before comparing.  For the Intel variant, only one
	 value of the high mantissa bit is valid for each exponent, so
	 this is not necessary.  */
      if (expx == 0x7fff)
	hx |= 0x80000000;
      if (expy == 0x7fff)
	hy |= 0x80000000;
    }
#if HIGH_ORDER_BIT_IS_SET_FOR_SNAN
# error not implemented
#endif
  return expx < expy || (expx == expy && (hx < hy || (hx == hy && lx <= ly)));
}
