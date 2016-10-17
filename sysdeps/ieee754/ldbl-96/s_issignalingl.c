/* Test for signaling NaN.
   Copyright (C) 2013-2016 Free Software Foundation, Inc.
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

int
__issignalingl (long double x)
{
  u_int32_t exi, hxi, lxi;
  GET_LDOUBLE_WORDS (exi, hxi, lxi, x);
#if HIGH_ORDER_BIT_IS_SET_FOR_SNAN
# error not implemented
#else
  /* To keep the following comparison simple, toggle the quiet/signaling bit,
     so that it is set for sNaNs.  This is inverse to IEEE 754-2008 (as well as
     common practice for IEEE 754-1985).  */
  hxi ^= 0x40000000;
  /* If lxi != 0, then set any suitable bit of the significand in hxi.  */
  hxi |= (lxi | -lxi) >> 31;
  /* We do not recognize a pseudo NaN as sNaN; they're invalid on 80387 and
     later.  */
  /* We have to compare for greater (instead of greater or equal), because x's
     significand being all-zero designates infinity not NaN.  */
  return ((exi & 0x7fff) == 0x7fff) && (hxi > 0xc0000000);
#endif
}
libm_hidden_def (__issignalingl)
