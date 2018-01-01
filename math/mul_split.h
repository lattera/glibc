/* Compute full X * Y for double type.
   Copyright (C) 2013-2018 Free Software Foundation, Inc.
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

#ifndef _MUL_SPLIT_H
#define _MUL_SPLIT_H

#include <float.h>

/* Calculate X * Y exactly and store the result in *HI + *LO.  It is
   given that the values are small enough that no overflow occurs and
   large enough (or zero) that no underflow occurs.  */

static void
mul_split (double *hi, double *lo, double x, double y)
{
#ifdef __FP_FAST_FMA
  /* Fast built-in fused multiply-add.  */
  *hi = x * y;
  *lo = __builtin_fma (x, y, -*hi);
#else
  /* Apply Dekker's algorithm.  */
  *hi = x * y;
# define C ((1 << (DBL_MANT_DIG + 1) / 2) + 1)
  double x1 = x * C;
  double y1 = y * C;
# undef C
  x1 = (x - x1) + x1;
  y1 = (y - y1) + y1;
  double x2 = x - x1;
  double y2 = y - y1;
  *lo = (((x1 * y1 - *hi) + x1 * y2) + x2 * y1) + x2 * y2;
#endif
}

#endif /* _MUL_SPLIT_H */
