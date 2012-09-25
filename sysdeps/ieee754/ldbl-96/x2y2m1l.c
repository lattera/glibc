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
add_split (long double *hi, long double *lo, long double x, long double y)
{
  /* Apply Dekker's algorithm.  */
  *hi = x + y;
  *lo = (x - *hi) + y;
}

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

/* Compare absolute values of floating-point values pointed to by P
   and Q for qsort.  */

static int
compare (const void *p, const void *q)
{
  long double pld = fabsl (*(const long double *) p);
  long double qld = fabsl (*(const long double *) q);
  if (pld < qld)
    return -1;
  else if (pld == qld)
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
  long double vals[4];
  SET_RESTORE_ROUNDL (FE_TONEAREST);
  mul_split (&vals[1], &vals[0], x, x);
  mul_split (&vals[3], &vals[2], y, y);
  if (x >= 0.75L)
    vals[1] -= 1.0L;
  else
    {
      vals[1] -= 0.5L;
      vals[3] -= 0.5L;
    }
  qsort (vals, 4, sizeof (long double), compare);
  /* Add up the values so that each element of VALS has absolute value
     at most equal to the last set bit of the next nonzero
     element.  */
  for (size_t i = 0; i <= 2; i++)
    {
      add_split (&vals[i + 1], &vals[i], vals[i + 1], vals[i]);
      qsort (vals + i + 1, 3 - i, sizeof (long double), compare);
    }
  /* Now any error from this addition will be small.  */
  return vals[3] + vals[2] + vals[1] + vals[0];
}
