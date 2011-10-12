/* w_lgammal.c -- long double version of w_lgamma.c.
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

/* long double lgammal(long double x)
 * Return the logarithm of the Gamma function of x.
 *
 * Method: call __ieee754_lgammal_r
 */

#include <math.h>
#include <math_private.h>

long double
__lgammal(long double x)
{
	int local_signgam = 0;
	long double y = __ieee754_lgammal_r(x,
					    _LIB_VERSION != _ISOC_
					    /* ISO C99 does not define the
					       global variable.  */
					    ? &signgam
					    : &local_signgam);
	if(__builtin_expect(!__finitel(y), 0)
	   && __finitel(x) && _LIB_VERSION != _IEEE_)
		return __kernel_standard(x, x,
					 __floorl(x)==x&&x<=0.0L
					 ? 215 /* lgamma pole */
					 : 214); /* lgamma overflow */

	return y;
}
weak_alias (__lgammal, lgammal)
strong_alias (__lgammal, __gammal)
weak_alias (__gammal, gammal)
