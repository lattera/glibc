/* s_significandl.c -- long double version of s_significand.c.
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
 * significandl(x) computes just
 * 	scalbl(x, (long double) -ilogbl(x)),
 * for exercising the fraction-part(F) IEEE 754-1985 test vector.
 */

#include "math.h"
#include "math_private.h"

#ifdef __STDC__
	long double __significandl(long double x)
#else
	long double __significandl(x)
	long double x;
#endif
{
	return __ieee754_scalbl(x,(long double) -ilogbl(x));
}
weak_alias (__significandl, significandl)
