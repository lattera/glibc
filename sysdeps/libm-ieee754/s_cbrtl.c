/* s_cbrtl.c -- long double version of s_cbrt.c.
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

#include "math.h"
#include "math_private.h"

/* cbrtl(x)
 * Return cube root of x
 */
#ifdef __STDC__
static const u_int32_t
#else
static u_int32_t
#endif
	B1_EXP = 10921,		/* = Int(B1) */
	B1_MANT = 0x7bc4b064,	/* = Int(1.0-0.03306235651)*2**31 */

	B2_EXP = 10900,
	B2_MANT = 0x7bc4b064;	/* = Int(1.0-0.03306235651)*2**31 */

#ifdef __STDC__
static const long double
#else
static long double
#endif
C =  5.42857142857142815906e-01L, /* 19/35 */
D = -7.05306122448979611050e-01L, /* -864/1225 */
E =  1.41428571428571436819e+00L, /* 99/70 */
F =  1.60714285714285720630e+00L, /* 45/28 */
G =  3.57142857142857150787e-01L; /* 5/14 */

#ifdef __STDC__
	long double __cbrtl(long double x)
#else
	long double __cbrtl(x)
	long double x;
#endif
{
	int32_t	hx;
	long double r,s,t=0.0,w;
	u_int32_t sign, se, x0, x1;

	GET_LDOUBLE_WORDS(se,x0,x1,x);
	sign=se&0x8000; 		/* sign= sign(x) */
	se ^= sign;
	if(se==0x7fff) return(x+x); /* cbrt(NaN,INF) is itself */
	if((se|x0|x1)==0)
	    return(x);		/* cbrt(0) is itself */

	SET_LDOUBLE_EXP(x,se);	/* x <- |x| */

/* XXX I don't know whether the numbers for correct are correct.  The
   precalculation is extended from 20 bits to 32 bits.  This hopefully
   gives us the needed bits to get us still along with one iteration
   step.  */

    /* rough cbrt to 5 bits */
	if(se==0) 		/* subnormal number */
	  {
	    u_int64_t xxl;
	    u_int32_t set,t0,t1;
	    SET_LDOUBLE_EXP(t,0x4035);	/* set t= 2**54 */
	    SET_LDOUBLE_MSW(t,0x80000000);
	    t*=x;
	    GET_LDOUBLE_WORDS(set,t0,t1,t);
	    xxl = ((u_int64_t) set) << 32 | t0;
	    xxl /= 3;
	    xxl += B2_EXP << 16 | B2_MANT;
	    t0 = xxl & 0xffffffffu;
	    set = xxl >> 32;
	    SET_LDOUBLE_WORDS(t,set,t0,t1);
	  }
	else
	  {
	    u_int64_t xxl = ((u_int64_t) se) << 32 | x0;
	    xxl /= 3;
	    xxl += B1_EXP << 16 | B1_MANT;
	    SET_LDOUBLE_MSW(t,xxl&0xffffffffu);
	    xxl >>= 32;
	    SET_LDOUBLE_EXP(t,xxl);
	  }


    /* new cbrt to 23 bits, may be implemented in single precision */
	r=t*t/x;
	s=C+r*t;
	t*=G+F/(s+E+D/s);

    /* chopped to 32 bits and make it larger than cbrt(x) */
	GET_LDOUBLE_WORDS(se,x0,x1,t);
	SET_LDOUBLE_WORDS(t,se,x0+1,0);


    /* one step newton iteration to 53 bits with error less than 0.667 ulps */
	s=t*t;		/* t*t is exact */
	r=x/s;
	w=t+t;
	r=(r-t)/(w+r);	/* r-s is exact */
	t=t+t*r;

    /* retore the sign bit */
	GET_LDOUBLE_EXP(se,t);
	SET_LDOUBLE_EXP(t,se|sign);
	return(t);
}
weak_alias (__cbrtl, cbrtl)
