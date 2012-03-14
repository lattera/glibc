/* @(#)e_hypotl.c 5.1 93/09/24 */
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

/* __ieee754_hypotl(x,y)
 *
 * Method :
 *	If (assume round-to-nearest) z=x*x+y*y
 *	has error less than sqrtl(2)/2 ulp, than
 *	sqrtl(z) has error less than 1 ulp (exercise).
 *
 *	So, compute sqrtl(x*x+y*y) with some care as
 *	follows to get the error below 1 ulp:
 *
 *	Assume x>y>0;
 *	(if possible, set rounding to round-to-nearest)
 *	1. if x > 2y  use
 *		x1*x1+(y*y+(x2*(x+x1))) for x*x+y*y
 *	where x1 = x with lower 53 bits cleared, x2 = x-x1; else
 *	2. if x <= 2y use
 *		t1*y1+((x-y)*(x-y)+(t1*y2+t2*y))
 *	where t1 = 2x with lower 53 bits cleared, t2 = 2x-t1,
 *	y1= y with lower 53 bits chopped, y2 = y-y1.
 *
 *	NOTE: scaling may be necessary if some argument is too
 *	      large or too tiny
 *
 * Special cases:
 *	hypotl(x,y) is INF if x or y is +INF or -INF; else
 *	hypotl(x,y) is NAN if x or y is NAN.
 *
 * Accuracy:
 *	hypotl(x,y) returns sqrtl(x^2+y^2) with error less
 *	than 1 ulps (units in the last place)
 */

#include <math.h>
#include <math_private.h>

static const long double two600 = 0x1.0p+600L;
static const long double two1022 = 0x1.0p+1022L;

long double
__ieee754_hypotl(long double x, long double y)
{
	long double a,b,t1,t2,y1,y2,w,kld;
	int64_t j,k,ha,hb;

	GET_LDOUBLE_MSW64(ha,x);
	ha &= 0x7fffffffffffffffLL;
	GET_LDOUBLE_MSW64(hb,y);
	hb &= 0x7fffffffffffffffLL;
	if(hb > ha) {a=y;b=x;j=ha; ha=hb;hb=j;} else {a=x;b=y;}
	a = fabsl(a);	/* a <- |a| */
	b = fabsl(b);	/* b <- |b| */
	if((ha-hb)>0x3c0000000000000LL) {return a+b;} /* x/y > 2**60 */
	k=0;
	kld = 1.0L;
	if(ha > 0x5f30000000000000LL) {	/* a>2**500 */
	   if(ha >= 0x7ff0000000000000LL) {	/* Inf or NaN */
	       u_int64_t low;
	       w = a+b;			/* for sNaN */
	       GET_LDOUBLE_LSW64(low,a);
	       if(((ha&0xfffffffffffffLL)|(low&0x7fffffffffffffffLL))==0)
		 w = a;
	       GET_LDOUBLE_LSW64(low,b);
	       if(((hb^0x7ff0000000000000LL)|(low&0x7fffffffffffffffLL))==0)
		 w = b;
	       return w;
	   }
	   /* scale a and b by 2**-600 */
	   ha -= 0x2580000000000000LL; hb -= 0x2580000000000000LL; k += 600;
	   a /= two600;
	   b /= two600;
	   k += 600;
	   kld = two600;
	}
	if(hb < 0x20b0000000000000LL) {	/* b < 2**-500 */
	    if(hb <= 0x000fffffffffffffLL) {	/* subnormal b or 0 */
		u_int64_t low;
		GET_LDOUBLE_LSW64(low,b);
		if((hb|(low&0x7fffffffffffffffLL))==0) return a;
		t1=two1022;	/* t1=2^1022 */
		b *= t1;
		a *= t1;
		k -= 1022;
		kld = kld / two1022;
	    } else {		/* scale a and b by 2^600 */
		ha += 0x2580000000000000LL;	/* a *= 2^600 */
		hb += 0x2580000000000000LL;	/* b *= 2^600 */
		k -= 600;
		a *= two600;
		b *= two600;
		kld = kld / two600;
	    }
	}
    /* medium size a and b */
	w = a-b;
	if (w>b) {
	    SET_LDOUBLE_WORDS64(t1,ha,0);
	    t2 = a-t1;
	    w  = __ieee754_sqrtl(t1*t1-(b*(-b)-t2*(a+t1)));
	} else {
	    a  = a+a;
	    SET_LDOUBLE_WORDS64(y1,hb,0);
	    y2 = b - y1;
	    SET_LDOUBLE_WORDS64(t1,ha+0x0010000000000000LL,0);
	    t2 = a - t1;
	    w  = __ieee754_sqrtl(t1*y1-(w*(-w)-(t1*y2+t2*b)));
	}
	if(k!=0)
	    return w*kld;
	else
	    return w;
}
strong_alias (__ieee754_hypotl, __hypotl_finite)
