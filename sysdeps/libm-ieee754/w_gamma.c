/* @(#)w_gamma.c 5.1 93/09/24 */
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
static char rcsid[] = "$NetBSD: w_gamma.c,v 1.7 1995/11/20 22:06:43 jtc Exp $";
#endif

/* double gamma(double x)
 * Return  the logarithm of the Gamma function of x or the Gamma function of x,
 * depending on the library mode.
 */

#include "math.h"
#include "math_private.h"

#ifdef __STDC__
	double __gamma(double x)
#else
	double __gamma(x)
	double x;
#endif
{
        double y;
#ifndef _IEEE_LIBM
	if (_LIB_VERSION == _SVID_)
	  y = __ieee754_lgamma_r(x,&signgam);
	else
	  {
#endif
	    int local_signgam;
	    y = __ieee754_gamma_r(x,&local_signgam);
	    if (local_signgam < 0) y = -y;
#ifdef _IEEE_LIBM
	    return y;
#else
	    if(_LIB_VERSION == _IEEE_) return y;
	  }
        if(!__finite(y)&&__finite(x)) {
            if(__floor(x)==x&&x<=0.0)
                return __kernel_standard(x,x,41); /* gamma pole */
            else
                return __kernel_standard(x,x,40); /* gamma overflow */
        } else
            return y;
#endif
}
weak_alias (__gamma, gamma)
#ifdef NO_LONG_DOUBLE
strong_alias (__gamma, __gammal)
weak_alias (__gamma, gammal)
#endif
