/* @(#)e_cosh.c 5.1 93/09/24 */
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

/* __ieee754_cosh(x)
 * Method :
 * mathematically cosh(x) if defined to be (exp(x)+exp(-x))/2
 *	1. Replace x by |x| (cosh(x) = cosh(-x)).
 *	2.
 *							[ exp(x) - 1 ]^2
 *	    0        <= x <= ln2/2  :  cosh(x) := 1 + -------------------
 *							   2*exp(x)
 *
 *						  exp(x) +  1/exp(x)
 *	    ln2/2    <= x <= 22     :  cosh(x) := -------------------
 *							  2
 *	    22       <= x <= lnovft :  cosh(x) := exp(x)/2
 *	    lnovft   <= x <= ln2ovft:  cosh(x) := exp(x/2)/2 * exp(x/2)
 *	    ln2ovft  <  x	    :  cosh(x) := huge*huge (overflow)
 *
 * Special cases:
 *	cosh(x) is |x| if x is +INF, -INF, or NaN.
 *	only cosh(0)=1 is exact for finite x.
 */

#include "math.h"
#include "math_private.h"

static const long double one = 1.0L, half=0.5L, huge = 1.0e300L;

long double
__ieee754_coshl (long double x)
{
	long double t,w;
	int64_t ix;

    /* High word of |x|. */
	GET_LDOUBLE_MSW64(ix,x);
	ix &= 0x7fffffffffffffffLL;

    /* x is INF or NaN */
	if(ix>=0x7ff0000000000000LL) return x*x;

    /* |x| in [0,0.5*ln2], return 1+expm1(|x|)^2/(2*exp(|x|)) */
	if(ix<0x3fd62e42fefa39efLL) {
	    t = __expm1l(fabsl(x));
	    w = one+t;
	    if (ix<0x3c80000000000000LL) return w;	/* cosh(tiny) = 1 */
	    return one+(t*t)/(w+w);
	}

    /* |x| in [0.5*ln2,22], return (exp(|x|)+1/exp(|x|)/2; */
	if (ix < 0x4036000000000000LL) {
		t = __ieee754_expl(fabsl(x));
		return half*t+half/t;
	}

    /* |x| in [22, log(maxdouble)] return half*exp(|x|) */
	if (ix < 0x40862e42fefa39efLL)  return half*__ieee754_expl(fabsl(x));

    /* |x| in [log(maxdouble), overflowthresold] */
	if (ix < 0x408633ce8fb9f87dLL) {
	    w = __ieee754_expl(half*fabsl(x));
	    t = half*w;
	    return t*w;
	}

    /* |x| > overflowthresold, cosh(x) overflow */
	return huge*huge;
}
strong_alias (__ieee754_coshl, __coshl_finite)
