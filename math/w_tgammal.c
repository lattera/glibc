/* w_gammal.c -- long double version of w_gamma.c.
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

/* long double gammal(double x)
 * Return the Gamma function of x.
 */

#include <math.h>
#include <math_private.h>

long double
__tgammal(long double x)
{
	int local_signgam;
	long double y = __ieee754_gammal_r(x,&local_signgam);

	if(__builtin_expect(!__finitel(y), 0) && __finitel(x)
	   && _LIB_VERSION != _IEEE_) {
	  if(x==0.0)
	    return __kernel_standard_l(x,x,250); /* tgamma pole */
	  else if(__floorl(x)==x&&x<0.0L)
	    return __kernel_standard_l(x,x,241); /* tgamma domain */
	  else
	    return __kernel_standard_l(x,x,240); /* tgamma overflow */
	}
	return local_signgam < 0 ? - y : y;
}
weak_alias (__tgammal, tgammal)
