/* e_fmodl.c -- long double version of e_fmod.c.
 * Conversion to IEEE quad long double by Jakub Jelinek, jj@ultra.linux.cz.
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
 *	Based on fmodl() return x-[x/p]chopped*p exactlp.
 */

#include "math.h"
#include "math_private.h"

static const long double zero = 0.0L;


long double
__ieee754_remainderl(long double x, long double p)
{
	int64_t hx,hp;
	u_int64_t sx,lx,lp;
	long double p_half;

	GET_LDOUBLE_WORDS64(hx,lx,x);
	GET_LDOUBLE_WORDS64(hp,lp,p);
	sx = hx&0x8000000000000000ULL;
	hp &= 0x7fffffffffffffffLL;
	hx &= 0x7fffffffffffffffLL;

    /* purge off exception values */
	if((hp|(lp&0x7fffffffffffffff))==0) return (x*p)/(x*p);	/* p = 0 */
	if((hx>=0x7ff0000000000000LL)||			/* x not finite */
	  ((hp>=0x7ff0000000000000LL)&&			/* p is NaN */
	  (((hp-0x7ff0000000000000LL)|lp)!=0)))
	    return (x*p)/(x*p);


	if (hp<=0x7fdfffffffffffffLL) x = __ieee754_fmodl(x,p+p);	/* now x < 2p */
	if (((hx-hp)|(lx-lp))==0) return zero*x;
	x  = fabsl(x);
	p  = fabsl(p);
	if (hp<0x0020000000000000LL) {
	    if(x+x>p) {
		x-=p;
		if(x+x>=p) x -= p;
	    }
	} else {
	    p_half = 0.5L*p;
	    if(x>p_half) {
		x-=p;
		if(x>=p_half) x -= p;
	    }
	}
	GET_LDOUBLE_MSW64(hx,x);
	SET_LDOUBLE_MSW64(x,hx^sx);
	return x;
}
strong_alias (__ieee754_remainderl, __remainderl_finite)
