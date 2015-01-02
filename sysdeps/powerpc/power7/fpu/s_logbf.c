/* logbf(). PowerPC/POWER7 version.
   Copyright (C) 2012-2015 Free Software Foundation, Inc.
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

#include "math_private.h"

/* This implementation avoids FP to INT conversions by using VSX
   bitwise instructions over FP values.  */

static const double two1div52 = 2.220446049250313e-16;	/* 1/2**52  */
static const double two10m1   = -1023.0;		/* -2**10 + 1  */
static const double two7m1    = -127.0;			/* -2**7 + 1  */

/* FP mask to extract the exponent.  */
static const union {
  unsigned long long mask;
  double d;
} mask = { 0x7ff0000000000000ULL };

float
__logbf (float x)
{
  /* VSX operation are all done internally as double.  */
  double ret;

  if (__builtin_expect (x == 0.0, 0))
    /* Raise FE_DIVBYZERO and return -HUGE_VAL[LF].  */
    return -1.0 / __builtin_fabsf (x);

  /* ret = x & 0x7f800000;  */
  asm (
    "xxland %x0,%x1,%x2\n"
    "fcfid  %0,%0"
    : "=f"(ret)
    : "f" (x), "f" (mask.d));
  /* ret = (ret >> 52) - 1023.0, since ret is double.  */
  ret = (ret * two1div52) + two10m1;
  if (__builtin_expect (ret > -two7m1, 0))
    /* Multiplication is used to set logb (+-INF) = INF.  */
    return (x * x);
  /* Since operations are done with double we don't need
     additional tests for subnormal numbers.
     The test is to avoid logb_downward (0.0) == -0.0.  */
  return ret == -0.0 ? 0.0 : ret;
}
weak_alias (__logbf, logbf)
