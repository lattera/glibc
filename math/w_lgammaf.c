/* w_lgammaf.c -- float version of w_lgamma.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
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

#include <math.h>
#include <math_private.h>

float
__lgammaf(float x)
{
	int local_signgam = 0;
	float y = __ieee754_lgammaf_r(x,
				      _LIB_VERSION != _ISOC_
				      /* ISO C99 does not define the
					 global variable.  */
				      ? &signgam
				      : &local_signgam);
	if(__builtin_expect(!__finitef(y), 0)
	   && __finitef(x) && _LIB_VERSION != _IEEE_)
		return __kernel_standard_f(x, x,
					   __floorf(x)==x&&x<=0.0f
					   ? 115 /* lgamma pole */
					   : 114); /* lgamma overflow */

	return y;
}
weak_alias (__lgammaf, lgammaf)
strong_alias (__lgammaf, __gammaf)
weak_alias (__gammaf, gammaf)
