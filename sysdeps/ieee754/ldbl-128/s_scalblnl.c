/* s_scalblnl.c -- long double version of s_scalbn.c.
 * Conversion to IEEE quad long double by Jakub Jelinek, jj@ultra.linux.cz.
 */
   
/* @(#)s_scalbn.c 5.1 93/09/24 */
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
 * scalblnl (long double x, long int n)
 * scalblnl(x,n) returns x* 2**n  computed by  exponent
 * manipulation rather than by actually performing an
 * exponentiation or a multiplication.
 */

#include "math.h"
#include "math_private.h"

#ifdef __STDC__
static const long double
#else
static long double
#endif
two114 = 2.0769187434139310514121985316880384E+34L, /* 0x4071000000000000, 0 */
twom114 = 4.8148248609680896326399448564623183E-35L, /* 0x3F8D000000000000, 0 */
huge   = 1.0E+4900L,
tiny   = 1.0E-4900L;

#ifdef __STDC__
	long double __scalblnl (long double x, long int n)
#else
	long double __scalblnl (x,n)
	long double x; long int n;
#endif
{
	int64_t k,hx,lx;
	GET_LDOUBLE_WORDS64(hx,lx,x);
        k = (hx>>48)&0x7fff;		/* extract exponent */
        if (k==0) {				/* 0 or subnormal x */
            if ((lx|(hx&0x7fffffffffffffffULL))==0) return x; /* +-0 */
	    x *= two114;
	    GET_LDOUBLE_MSW64(hx,x);
	    k = ((hx>>48)&0x7fff) - 114;
	}
        if (k==0x7fff) return x+x;		/* NaN or Inf */
        k = k+n;
        if (n> 50000 || k > 0x7ffe)
	  return huge*__copysignl(huge,x); /* overflow  */
	if (n< -50000) return tiny*__copysignl(tiny,x); /*underflow*/
        if (k > 0) 				/* normal result */
	    {SET_LDOUBLE_MSW64(x,(hx&0x8000ffffffffffffULL)|(k<<48)); return x;}
        if (k <= -114)
	  return tiny*__copysignl(tiny,x); 	/*underflow*/
        k += 114;				/* subnormal result */
	SET_LDOUBLE_MSW64(x,(hx&0x8000ffffffffffffULL)|(k<<48));
        return x*twom114;
}
weak_alias (__scalblnl, scalblnl)
