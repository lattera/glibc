/* Get NaN payload.  ldbl-96 version.
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
#include <stdint.h>

long double
getpayloadl (const long double *x)
{
  uint16_t se __attribute__ ((unused));
  uint32_t hx, lx;
  GET_LDOUBLE_WORDS (se, hx, lx, *x);
  hx &= 0x3fffffff;
  uint64_t ix = ((uint64_t) hx << 32) | lx;
  return (long double) ix;
}
