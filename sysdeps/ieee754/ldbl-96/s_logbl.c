/* s_logbl.c -- long double version of s_logb.c.
 * Conversion to long double by Ulrich Drepper,
 * Cygnus Support, drepper@cygnus.com.
 */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * long double logbl(x)
 * IEEE 754 logb. Included to pass IEEE test suite. Not recommend.
 * Use ilogb instead.
 */

#include <math.h>
#include <math_private.h>

long double
__logbl (long double x)
{
  int32_t es, lx, ix;

  GET_LDOUBLE_WORDS (es, ix, lx, x);
  es &= 0x7fff;			/* exponent */
  if ((es | ix | lx) == 0)
    return -1.0 / fabs (x);
  if (es == 0x7fff)
    return x * x;
  if (es == 0)			/* IEEE 754 logb */
    {
      /* POSIX specifies that denormal number is treated as
         though it were normalized.  */
      int ma;
      if (ix == 0)
	ma = __builtin_clz (lx) + 32;
      else
	ma = __builtin_clz (ix);
      es -= ma - 1;
    }
  return (long double) (es - 16383);
}

weak_alias (__logbl, logbl)
