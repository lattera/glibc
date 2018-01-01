/* logbl(). PowerPC/POWER7 version.
   Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

long double
__logbl (long double x)
{
  double xh, xl;
  double ret;
  int64_t hx;

  if (__builtin_expect (x == 0.0L, 0))
    /* Raise FE_DIVBYZERO and return -HUGE_VAL[LF].  */
    return -1.0L / __builtin_fabsl (x);

  ldbl_unpack (x, &xh, &xl);
  EXTRACT_WORDS64 (hx, xh);
  /* ret = x & 0x7ff0000000000000;  */
  asm (
    "xxland %x0,%x1,%x2\n"
    "fcfid  %0,%0"
    : "=f" (ret)
    : "f" (xh), "f" (mask.d));
  /* ret = (ret >> 52) - 1023.0;  */
  ret = (ret * two1div52) + two10m1;
  if (__builtin_expect (ret > -two10m1, 0))
    /* Multiplication is used to set logb (+-INF) = INF.  */
    return (xh * xh);
  else if (__builtin_expect (ret == two10m1, 0))
    {
      /* POSIX specifies that denormal number is treated as
         though it were normalized.  */
      return (long double) (- (__builtin_clzll (hx & 0x7fffffffffffffffLL) \
			       - 12) - 1023);
    }
  else if ((hx & 0x000fffffffffffffLL) == 0)
    {
      /* If the high part is a power of 2, and the low part is nonzero
	 with the opposite sign, the low part affects the
	 exponent.  */
      int64_t lx, rhx;
      EXTRACT_WORDS64 (lx, xl);
      rhx = (hx & 0x7ff0000000000000LL) >> 52;
      if ((hx ^ lx) < 0 && (lx & 0x7fffffffffffffffLL) != 0)
	rhx--;
      return (long double) (rhx - 1023);
    }
  /* Test to avoid logb_downward (0.0) == -0.0.  */
  return ret == -0.0 ? 0.0 : ret;
}
#ifndef __logbl
long_double_symbol (libm, __logbl, logbl);
#endif
