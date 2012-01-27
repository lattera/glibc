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

#include "math.h"
#include "math_private.h"

long double __logbl(long double x)
{
	int64_t lx,hx;
	GET_LDOUBLE_WORDS64(hx,lx,x);
	hx &= 0x7fffffffffffffffLL;		/* high |x| */
	if((hx|lx)==0) return -1.0/fabs(x);
	if(hx>=0x7fff000000000000LL) return x*x;
	if((hx>>=48)==0) 			/* IEEE 754 logb */
		return -16382.0;
	else
		return (long double) (hx-0x3fff);
}
weak_alias (__logbl, logbl)
