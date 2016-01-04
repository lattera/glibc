/* Compute a product of 1 + (T/X), 1 + (T/(X+1)), ....
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

static void
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

/* Compute the product of 1 + (T / (X + X_EPS)), 1 + (T / (X + X_EPS +
   1)), ..., 1 + (T / (X + X_EPS + N - 1)), minus 1.  X is such that
   all the values X + 1, ..., X + N - 1 are exactly representable, and
   X_EPS / X is small enough that factors quadratic in it can be
   neglected.  */

long double
__lgamma_productl (long double t, long double x, long double x_eps, int n)
{
  long double ret = 0, ret_eps = 0;
  for (int i = 0; i < n; i++)
    {
      long double xi = x + i;
      long double quot = t / xi;
      long double mhi, mlo;
      mul_split (&mhi, &mlo, quot, xi);
      long double quot_lo = (t - mhi - mlo) / xi - t * x_eps / (xi * xi);
      /* We want (1 + RET + RET_EPS) * (1 + QUOT + QUOT_LO) - 1.  */
      long double rhi, rlo;
      mul_split (&rhi, &rlo, ret, quot);
      long double rpq = ret + quot;
      long double rpq_eps = (ret - rpq) + quot;
      long double nret = rpq + rhi;
      long double nret_eps = (rpq - nret) + rhi;
      ret_eps += (rpq_eps + nret_eps + rlo + ret_eps * quot
		  + quot_lo + quot_lo * (ret + ret_eps));
      ret = nret;
    }
  return ret + ret_eps;
}
