/* @(#)w_lgamma.c 5.1 93/09/24 */
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

/* double lgamma(double x)
 * Return the logarithm of the Gamma function of x.
 *
 * Method: call __ieee754_lgamma_r
 */

#include <math.h>
#include <math_private.h>

double
__lgamma(double x)
{
	int local_signgam = 0;
	double y = __ieee754_lgamma_r(x,
				      _LIB_VERSION != _ISOC_
				      /* ISO C99 does not define the
					 global variable.  */
				      ? &signgam
				      : &local_signgam);
	if(__builtin_expect(!__finite(y), 0)
	   && __finite(x) && _LIB_VERSION != _IEEE_)
		return __kernel_standard(x, x,
					 __floor(x)==x&&x<=0.0
					 ? 15 /* lgamma pole */
					 : 14); /* lgamma overflow */

	return y;
}
weak_alias (__lgamma, lgamma)
strong_alias (__lgamma, __gamma)
weak_alias (__gamma, gamma)
#ifdef NO_LONG_DOUBLE
strong_alias (__lgamma, __lgammal)
weak_alias (__lgamma, lgammal)
strong_alias (__gamma, __gammal)
weak_alias (__gamma, gammal)
#endif
