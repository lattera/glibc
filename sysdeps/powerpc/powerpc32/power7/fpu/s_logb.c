/* logb(). PowerPC/POWER7 version.
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

#include <math_ldbl_opt.h>

/* This implementation avoids FP to INT conversions by using VSX
   bitwise instructions over FP values.  */

static const double two1div52 = 2.220446049250313e-16;	/* 1/2**52  */
static const double two10m1   = -1023.0;		/* 2**10 -1  */

/* FP mask to extract the exponent.  */
static const union {
  unsigned long long mask;
  double d;
} mask = { 0x7ff0000000000000ULL };

double
__logb (double x)
{
  double ret;

  if (__builtin_expect (x == 0.0, 0))
    /* Raise FE_DIVBYZERO and return -HUGE_VAL[LF].  */
    return -1.0 / __builtin_fabs (x);

  /* ret = x & 0x7ff0000000000000;  */
  asm (
    "xxland %x0,%x1,%x2\n"
    "fcfid  %0,%0"
    : "=f" (ret)
    : "f" (x), "f" (mask.d));
  /* ret = (ret >> 52) - 1023.0;  */
  ret = (ret * two1div52) + two10m1;
  if (__builtin_expect (ret > -two10m1, 0))
    /* Multiplication is used to set logb (+-INF) = INF.  */
    return (x * x);
  else if (__builtin_expect (ret == two10m1, 0))
    {
      /* POSIX specifies that denormal numbers are treated as
         though they were normalized.  */
      int32_t lx, ix;
      int ma;

      EXTRACT_WORDS (ix, lx, x);
      if (ix == 0)
	ma = __builtin_clz (lx) + 32;
      else
	ma = __builtin_clz (ix);
      return (double) (-1023 - (ma - 12));
    }
  /* Test to avoid logb_downward (0.0) == -0.0.  */
  return ret == -0.0 ? 0.0 : ret;
}
weak_alias (__logb, logb)
#ifdef NO_LONG_DOUBLE
strong_alias (__logb, __logbl)
weak_alias (__logb, logbl)
#endif

#if LONG_DOUBLE_COMPAT (libm, GLIBC_2_0)
compat_symbol (libm, logb, logbl, GLIBC_2_0);
#endif
