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

/* __ieee754_coshl(x)
 * Method :
 * mathematically coshl(x) if defined to be (exp(x)+exp(-x))/2
 *      1. Replace x by |x| (coshl(x) = coshl(-x)).
 *      2.
 *                                                      [ exp(x) - 1 ]^2
 *          0        <= x <= ln2/2  :  coshl(x) := 1 + -------------------
 *                                                         2*exp(x)
 *
 *                                                 exp(x) +  1/exp(x)
 *          ln2/2    <= x <= 22     :  coshl(x) := -------------------
 *                                                         2
 *          22       <= x <= lnovft :  coshl(x) := expl(x)/2
 *          lnovft   <= x <= ln2ovft:  coshl(x) := expl(x/2)/2 * expl(x/2)
 *          ln2ovft  <  x           :  coshl(x) := huge*huge (overflow)
 *
 * Special cases:
 *      coshl(x) is |x| if x is +INF, -INF, or NaN.
 *      only coshl(0)=1 is exact for finite x.
 */

#include "math.h"
#include "math_private.h"

#ifdef __STDC__
static const long double one = 1.0, half = 0.5, huge = 1.0e4900L,
ovf_thresh = 1.1357216553474703894801348310092223067821E4L;
#else
static long double one = 1.0, half = 0.5, huge = 1.0e4900L,
ovf_thresh = 1.1357216553474703894801348310092223067821E4L;
#endif

#ifdef __STDC__
long double
__ieee754_coshl (long double x)
#else
long double
__ieee754_coshl (x)
     long double x;
#endif
{
  long double t, w;
  int32_t ex;
  u_int32_t mx, lx;
  ieee854_long_double_shape_type u;

  u.value = x;
  ex = u.parts32.w0 & 0x7fffffff;

  /* Absolute value of x.  */
  u.parts32.w0 = ex;

  /* x is INF or NaN */
  if (ex >= 0x7fff0000)
    return x * x;

  /* |x| in [0,0.5*ln2], return 1+expm1l(|x|)^2/(2*expl(|x|)) */
  if (ex < 0x3ffd62e4) /* 0.3465728759765625 */
    {
      t = __expm1l (u.value);
      w = one + t;
      if (ex < 0x3fc60000) /* |x| < 2^-57 */
	return w;		/* cosh(tiny) = 1 */

      return one + (t * t) / (w + w);
    }

  /* |x| in [0.5*ln2,40], return (exp(|x|)+1/exp(|x|)/2; */
  if (ex < 0x40044000)
    {
      t = __ieee754_expl (u.value);
      return half * t + half / t;
    }

  /* |x| in [22, ln(maxdouble)] return half*exp(|x|) */
  if (ex <= 0x400c62e3) /* 11356.375 */
    return half * __ieee754_expl (u.value);

  /* |x| in [log(maxdouble), overflowthresold] */
  if (u.value <= ovf_thresh)
    {
      w = __ieee754_expl (half * u.value);
      t = half * w;
      return t * w;
    }

  /* |x| > overflowthresold, cosh(x) overflow */
  return huge * huge;
}
