/* @(#)e_atanh.c 5.1 93/09/24 */
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

/* __ieee754_atanh(x)
 * Method :
 *    1.Reduced x to positive by atanh(-x) = -atanh(x)
 *    2.For x>=0.5
 *                  1              2x                          x
 *	atanh(x) = --- * log(1 + -------) = 0.5 * log1p(2 * --------)
 *                  2             1 - x                      1 - x
 *
 *	For x<0.5
 *	atanh(x) = 0.5*log1p(2x+2x*x/(1-x))
 *
 * Special cases:
 *	atanh(x) is NaN if |x| > 1 with signal;
 *	atanh(NaN) is that NaN with no signal;
 *	atanh(+-1) is +-INF with signal.
 *
 */

#include <math.h>
#include <math_private.h>

static const long double one = 1.0L, huge = 1e300L;

static const long double zero = 0.0L;

long double
__ieee754_atanhl(long double x)
{
	long double t;
	int64_t hx,ix;
	u_int64_t lx;
	GET_LDOUBLE_WORDS64(hx,lx,x);
	ix = hx&0x7fffffffffffffffLL;
	if (ix >= 0x3ff0000000000000LL) { /* |x|>=1 */
	    if (ix > 0x3ff0000000000000LL)
		return (x-x)/(x-x);
	    t = fabsl (x);
	    if (t > one)
		return (x-x)/(x-x);
	    if (t == one)
		return x/zero;
	}
	if(ix<0x3e20000000000000LL&&(huge+x)>zero) return x;	/* x<2**-29 */
	x = fabsl (x);
	if(ix<0x3fe0000000000000LL) {		/* x < 0.5 */
	    t = x+x;
	    t = 0.5*__log1pl(t+t*x/(one-x));
	} else
	    t = 0.5*__log1pl((x+x)/(one-x));
	if(hx>=0) return t; else return -t;
}
strong_alias (__ieee754_atanhl, __atanhl_finite)
