/* Get NaN payload.  dbl-64/wordsize-64 version.
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

double
getpayload (const double *x)
{
  uint64_t ix;
  EXTRACT_WORDS64 (ix, *x);
  ix &= 0x7ffffffffffffULL;
  return (double) ix;
}
#ifdef NO_LONG_DOUBLE
weak_alias (getpayload, getpayloadl)
#endif
