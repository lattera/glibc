/* Get NaN payload.  dbl-64 version.
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

#include <fix-int-fp-convert-zero.h>
#include <math.h>
#include <math_private.h>
#include <libm-alias-double.h>
#include <stdint.h>

double
__getpayload (const double *x)
{
  uint32_t hx, lx;
  EXTRACT_WORDS (hx, lx, *x);
  hx &= 0x7ffff;
  uint64_t ix = ((uint64_t) hx << 32) | lx;
  if (FIX_INT_FP_CONVERT_ZERO && ix == 0)
    return 0.0f;
  return (double) ix;
}
libm_alias_double (__getpayload, getpayload)
