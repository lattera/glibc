/* s_logbl.c -- long double version of s_logb.c.
 * Conversion to IEEE quad long double by Jakub Jelinek, jj@ultra.linux.cz.
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

#if defined(LIBM_SCCS) && !defined(lint)
static char rcsid[] = "$NetBSD: $";
#endif

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
  int64_t lx, hx, ex;

  GET_LDOUBLE_WORDS64 (hx, lx, x);
  hx &= 0x7fffffffffffffffLL;	/* high |x| */
  if ((hx | lx) == 0)
    return -1.0 / fabs (x);
  if (hx >= 0x7fff000000000000LL)
    return x * x;
  if ((ex = hx >> 48) == 0)	/* IEEE 754 logb */
    {
      /* POSIX specifies that denormal number is treated as
         though it were normalized.  */
      int m1 = (hx == 0) ? 0 : __builtin_clzll (hx);
      int m2 = (lx == 0) ? 0 : __builtin_clzll (lx);
      int ma = (m1 == 0) ? m2 + 64 : m1;
      return -16382.0 + (long double)(15 - ma);
    }
  return (long double) (ex - 16383);
}

weak_alias (__logbl, logbl)
