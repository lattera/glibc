/* w_expl.c -- long double version of w_exp.c.
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
 * wrapper expl(x)
 */

#include "math.h"
#include "math_private.h"

#ifdef __STDC__
static const long double
#else
static long double
#endif
o_threshold= 1.1356523406294143949491931077970763428449E4L,
u_threshold= -1.1433462743336297878837243843452621503410E4;

#ifdef __STDC__
	long double __expl(long double x)	/* wrapper exp */
#else
	long double __expl(x)			/* wrapper exp */
	long double x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_expl(x);
#else
	long double z;
	z = __ieee754_expl(x);
	if(_LIB_VERSION == _IEEE_) return z;
	if(__finitel(x)) {
	    if(x>o_threshold)
	        return __kernel_standard(x,x,206); /* exp overflow */
	    else if(x<u_threshold)
	        return __kernel_standard(x,x,207); /* exp underflow */
	}
	return z;
#endif
}
weak_alias (__expl, expl)
