/* s_scalbnl.c -- long double version of s_scalbn.c.
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
 * scalbnl (long double x, int n)
 * scalbnl(x,n) returns x* 2**n  computed by  exponent
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
two54   =  1.80143985094819840000e+16, /* 0x4035, 0x00000000, 0x00000000 */
twom54  =  5.55111512312578270212e-17, /* 0x3FC9, 0x00000000, 0x00000000 */
huge   = 1.0e+4900L,
tiny   = 1.0e-4900L;

#ifdef __STDC__
	long double __scalbnl (long double x, int n)
#else
	long double __scalbnl (x,n)
	long double x; int n;
#endif
{
	int32_t k,es,hx,lx;
	GET_LDOUBLE_WORDS(es,hx,lx,x);
        k = es&0x7fff;				/* extract exponent */
        if (k==0) {				/* 0 or subnormal x */
            if ((lx|(hx&0x7fffffff))==0) return x; /* +-0 */
	    x *= two54;
	    GET_HIGH_WORD(hx,x);
	    k = ((hx&0x7ff00000)>>20) - 54;
            if (n< -50000) return tiny*x; 	/*underflow*/
	    }
        if (k==0x7ff) return x+x;		/* NaN or Inf */
        k = k+n;
        if (k >  0x7fe) return huge*__copysign(huge,x); /* overflow  */
        if (k > 0) 				/* normal result */
	    {SET_HIGH_WORD(x,(hx&0x800fffff)|(k<<20)); return x;}
        if (k <= -54)
            if (n > 50000) 	/* in case integer overflow in n+k */
		return huge*__copysign(huge,x);	/*overflow*/
	    else return tiny*__copysign(tiny,x); 	/*underflow*/
        k += 54;				/* subnormal result */
	SET_HIGH_WORD(x,(hx&0x800fffff)|(k<<20));
        return x*twom54;
}
weak_alias (__scalbnl, scalbnl)
