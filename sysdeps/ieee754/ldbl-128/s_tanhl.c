/* s_tanhl.c -- long double version of s_tanh.c.
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

/* Changes for 128-bit long double contributed by
   Stephen L. Moshier <moshier@na-net.ornl.gov> */

/* tanhl(x)
 * Return the Hyperbolic Tangent of x
 *
 * Method :
 *                                      x    -x
 *                                     e  - e
 *      0. tanhl(x) is defined to be -----------
 *                                      x    -x
 *                                     e  + e
 *      1. reduce x to non-negative by tanhl(-x) = -tanhl(x).
 *      2.  0      <= x <= 2**-57 : tanhl(x) := x*(one+x)
 *                                               -t
 *          2**-57 <  x <=  1     : tanhl(x) := -----; t = expm1l(-2x)
 *                                              t + 2
 *                                                    2
 *          1      <= x <=  40.0  : tanhl(x) := 1-  ----- ; t=expm1l(2x)
 *                                                  t + 2
 *          40.0   <  x <= INF    : tanhl(x) := 1.
 *
 * Special cases:
 *      tanhl(NaN) is NaN;
 *      only tanhl(0)=0 is exact for finite argument.
 */

#include "math.h"
#include "math_private.h"

#ifdef __STDC__
static const long double one = 1.0, two = 2.0, tiny = 1.0e-4900L;
#else
static long double one = 1.0, two = 2.0, tiny = 1.0e-4900L;
#endif

#ifdef __STDC__
long double
__tanhl (long double x)
#else
long double
__tanhl (x)
     long double x;
#endif
{
  long double t, z;
  u_int32_t jx, ix;
  ieee854_long_double_shape_type u;

  /* Words of |x|. */
  u.value = x;
  jx = u.parts32.w0;
  ix = jx & 0x7fffffff;
  /* x is INF or NaN */
  if (ix >= 0x7fff0000)
    {
      /* for NaN it's not important which branch: tanhl(NaN) = NaN */
      if (jx & 0x80000000)
	return one / x - one;	/* tanhl(-inf)= -1; */
      else
	return one / x + one;	/* tanhl(+inf)=+1 */
    }

  /* |x| < 40 */
  if (ix < 0x40044000)
    {
      if (u.value == 0)
	return x;		/* x == +- 0 */
      if (ix < 0x3fc60000)	/* |x| < 2^-57 */
	return x * (one + tiny); /* tanh(small) = small */
      u.parts32.w0 = ix;	/* Absolute value of x.  */
      if (ix >= 0x3fff0000)
	{			/* |x| >= 1  */
	  t = __expm1l (two * u.value);
	  z = one - two / (t + two);
	}
      else
	{
	  t = __expm1l (-two * u.value);
	  z = -t / (t + two);
	}
      /* |x| > 40, return +-1 */
    }
  else
    {
      z = one - tiny;		/* raised inexact flag */
    }
  return (jx & 0x80000000) ? -z : z;
}
weak_alias (__tanhl, tanhl)
