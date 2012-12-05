/* Compute x^2 + y^2 - 1, without large cancellation error.
   Copyright (C) 2012 Free Software Foundation, Inc.
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
#include <stdlib.h>

/* Calculate X + Y exactly and store the result in *HI + *LO.  It is
   given that |X| >= |Y| and the values are small enough that no
   overflow occurs.  */

static inline void
add_split (double *hi, double *lo, double x, double y)
{
  /* Apply Dekker's algorithm.  */
  *hi = x + y;
  *lo = (x - *hi) + y;
}

/* Calculate X * Y exactly and store the result in *HI + *LO.  It is
   given that the values are small enough that no overflow occurs and
   large enough (or zero) that no underflow occurs.  */

static inline void
mul_split (double *hi, double *lo, double x, double y)
{
#ifdef __FP_FAST_FMA
  /* Fast built-in fused multiply-add.  */
  *hi = x * y;
  *lo = __builtin_fma (x, y, -*hi);
#elif defined FP_FAST_FMA
  /* Fast library fused multiply-add, compiler before GCC 4.6.  */
  *hi = x * y;
  *lo = __fma (x, y, -*hi);
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

/* Compare absolute values of floating-point values pointed to by P
   and Q for qsort.  */

static int
compare (const void *p, const void *q)
{
  double pd = fabs (*(const double *) p);
  double qd = fabs (*(const double *) q);
  if (pd < qd)
    return -1;
  else if (pd == qd)
    return 0;
  else
    return 1;
}

/* Return X^2 + Y^2 - 1, computed without large cancellation error.
   It is given that 1 > X >= Y >= epsilon / 2, and that either X >=
   0.75 or Y >= 0.5.  */

long double
__x2y2m1l (long double x, long double y)
{
  double vals[12];
  SET_RESTORE_ROUND (FE_TONEAREST);
  union ibm_extended_long_double xu, yu;
  xu.d = x;
  yu.d = y;
  if (fabs (xu.dd[1]) < 0x1p-500)
    xu.dd[1] = 0.0;
  if (fabs (yu.dd[1]) < 0x1p-500)
    yu.dd[1] = 0.0;
  mul_split (&vals[1], &vals[0], xu.dd[0], xu.dd[0]);
  mul_split (&vals[3], &vals[2], xu.dd[0], xu.dd[1]);
  vals[2] *= 2.0;
  vals[3] *= 2.0;
  mul_split (&vals[5], &vals[4], xu.dd[1], xu.dd[1]);
  mul_split (&vals[7], &vals[6], yu.dd[0], yu.dd[0]);
  mul_split (&vals[9], &vals[8], yu.dd[0], yu.dd[1]);
  vals[8] *= 2.0;
  vals[9] *= 2.0;
  mul_split (&vals[11], &vals[10], yu.dd[1], yu.dd[1]);
  if (xu.dd[0] >= 0.75)
    vals[1] -= 1.0;
  else
    {
      vals[1] -= 0.5;
      vals[7] -= 0.5;
    }
  qsort (vals, 12, sizeof (double), compare);
  /* Add up the values so that each element of VALS has absolute value
     at most equal to the last set bit of the next nonzero
     element.  */
  for (size_t i = 0; i <= 10; i++)
    {
      add_split (&vals[i + 1], &vals[i], vals[i + 1], vals[i]);
      qsort (vals + i + 1, 11 - i, sizeof (double), compare);
    }
  /* Now any error from this addition will be small.  */
  long double retval = (long double) vals[11];
  for (size_t i = 10; i != (size_t) -1; i--)
    retval += (long double) vals[i];
  return retval;
}
