/* e_remainderl.c -- long double version of e_remainder.c.
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

/* __ieee754_remainderl(x,p)
 * Return :
 *	returns  x REM p  =  x - [x/p]*p as if in infinite
 *	precise arithmetic, where [x/p] is the (infinite bit)
 *	integer nearest x/p (in half way case choose the even one).
 * Method :
 *	Based on fmod() return x-[x/p]chopped*p exactlp.
 */

#include "math.h"
#include "math_private.h"

static const long double zero = 0.0;


long double
__ieee754_remainderl(long double x, long double p)
{
	u_int32_t sx,sex,sep,x0,x1,p0,p1;
	long double p_half;

	GET_LDOUBLE_WORDS(sex,x0,x1,x);
	GET_LDOUBLE_WORDS(sep,p0,p1,p);
	sx = sex&0x8000;
	sep &= 0x7fff;
	sex &= 0x7fff;

    /* purge off exception values */
	if((sep|p0|p1)==0) return (x*p)/(x*p);	/* p = 0 */
	if((sex==0x7fff)||			/* x not finite */
	  ((sep==0x7fff)&&			/* p is NaN */
	   ((p0|p1)!=0)))
	    return (x*p)/(x*p);


	if (sep<0x7ffe) x = __ieee754_fmodl(x,p+p);	/* now x < 2p */
	if (((sex-sep)|(x0-p0)|(x1-p1))==0) return zero*x;
	x  = fabsl(x);
	p  = fabsl(p);
	if (sep<0x0002) {
	    if(x+x>p) {
		x-=p;
		if(x+x>=p) x -= p;
	    }
	} else {
	    p_half = 0.5*p;
	    if(x>p_half) {
		x-=p;
		if(x>=p_half) x -= p;
	    }
	}
	GET_LDOUBLE_EXP(sex,x);
	SET_LDOUBLE_EXP(x,sex^sx);
	return x;
}
strong_alias (__ieee754_remainderl, __remainderl_finite)
