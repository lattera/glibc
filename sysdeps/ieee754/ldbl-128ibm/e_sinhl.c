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


/* __ieee754_sinh(x)
 * Method :
 * mathematically sinh(x) if defined to be (exp(x)-exp(-x))/2
 *	1. Replace x by |x| (sinh(-x) = -sinh(x)).
 *	2.
 *						    E + E/(E+1)
 *	    0        <= x <= 22     :  sinh(x) := --------------, E=expm1(x)
 *							2
 *
 *	    22       <= x <= lnovft :  sinh(x) := exp(x)/2
 *	    lnovft   <= x <= ln2ovft:  sinh(x) := exp(x/2)/2 * exp(x/2)
 *	    ln2ovft  <  x	    :  sinh(x) := x*shuge (overflow)
 *
 * Special cases:
 *	sinh(x) is |x| if x is +INF, -INF, or NaN.
 *	only sinh(0)=0 is exact for finite x.
 */

#include "math.h"
#include "math_private.h"

static const long double one = 1.0, shuge = 1.0e307;

long double
__ieee754_sinhl(long double x)
{
	long double t,w,h;
	int64_t ix,jx;

    /* High word of |x|. */
	GET_LDOUBLE_MSW64(jx,x);
	ix = jx&0x7fffffffffffffffLL;

    /* x is INF or NaN */
	if(ix>=0x7ff0000000000000LL) return x+x;

	h = 0.5;
	if (jx<0) h = -h;
    /* |x| in [0,22], return sign(x)*0.5*(E+E/(E+1))) */
	if (ix < 0x4036000000000000LL) {	/* |x|<22 */
	    if (ix<0x3e20000000000000LL)	/* |x|<2**-29 */
		if(shuge+x>one) return x;/* sinhl(tiny) = tiny with inexact */
	    t = __expm1l(fabsl(x));
	    if(ix<0x3ff0000000000000LL) return h*(2.0*t-t*t/(t+one));
	    return h*(t+t/(t+one));
	}

    /* |x| in [22, log(maxdouble)] return 0.5*exp(|x|) */
	if (ix < 0x40862e42fefa39efLL)  return h*__ieee754_expl(fabsl(x));

    /* |x| in [log(maxdouble), overflowthresold] */
	if (ix <= 0x408633ce8fb9f87dLL) {
	    w = __ieee754_expl(0.5*fabsl(x));
	    t = h*w;
	    return t*w;
	}

    /* |x| > overflowthresold, sinh(x) overflow */
	return x*shuge;
}
strong_alias (__ieee754_sinhl, __sinhl_finite)
