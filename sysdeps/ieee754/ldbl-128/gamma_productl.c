/* Compute a product of X, X+1, ..., with an error estimate.
   Copyright (C) 2013-2015 Free Software Foundation, Inc.
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
#include <float.h>

/* Calculate X * Y exactly and store the result in *HI + *LO.  It is
   given that the values are small enough that no overflow occurs and
   large enough (or zero) that no underflow occurs.  */

static inline void
mul_split (long double *hi, long double *lo, long double x, long double y)
{
#ifdef __FP_FAST_FMAL
  /* Fast built-in fused multiply-add.  */
  *hi = x * y;
  *lo = __builtin_fmal (x, y, -*hi);
#elif defined FP_FAST_FMAL
  /* Fast library fused multiply-add, compiler before GCC 4.6.  */
  *hi = x * y;
  *lo = __fmal (x, y, -*hi);
#else
  /* Apply Dekker's algorithm.  */
  *hi = x * y;
# define C ((1LL << (LDBL_MANT_DIG + 1) / 2) + 1)
  long double x1 = x * C;
  long double y1 = y * C;
# undef C
  x1 = (x - x1) + x1;
  y1 = (y - y1) + y1;
  long double x2 = x - x1;
  long double y2 = y - y1;
  *lo = (((x1 * y1 - *hi) + x1 * y2) + x2 * y1) + x2 * y2;
#endif
}

/* Compute the product of X + X_EPS, X + X_EPS + 1, ..., X + X_EPS + N
   - 1, in the form R * (1 + *EPS) where the return value R is an
   approximation to the product and *EPS is set to indicate the
   approximate error in the return value.  X is such that all the
   values X + 1, ..., X + N - 1 are exactly representable, and X_EPS /
   X is small enough that factors quadratic in it can be
   neglected.  */

long double
__gamma_productl (long double x, long double x_eps, int n, long double *eps)
{
  SET_RESTORE_ROUNDL (FE_TONEAREST);
  long double ret = x;
  *eps = x_eps / x;
  for (int i = 1; i < n; i++)
    {
      *eps += x_eps / (x + i);
      long double lo;
      mul_split (&ret, &lo, ret, x + i);
      *eps += lo / ret;
    }
  return ret;
}
