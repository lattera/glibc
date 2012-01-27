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
	int32_t es,lx,ix;
	GET_LDOUBLE_WORDS(es,ix,lx,x);
	es &= 0x7fff;				/* exponent */
	if((es|ix|lx)==0) return -1.0/fabs(x);
	if(es==0x7fff) return x*x;
	if(es==0) 			/* IEEE 754 logb */
		return -16382.0;
	else
		return (long double) (es-0x3fff);
}
weak_alias (__logbl, logbl)
