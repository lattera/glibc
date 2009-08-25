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

/*
 * scalbn (double x, int n)
 * scalbn(x,n) returns x* 2**n  computed by  exponent
 * manipulation rather than by actually performing an
 * exponentiation or a multiplication.
 */

#include "math.h"
#include "math_private.h"

#ifdef __STDC__
static const double
#else
static double
#endif
two54   =  1.80143985094819840000e+16, /* 0x43500000, 0x00000000 */
twom54  =  5.55111512312578270212e-17, /* 0x3C900000, 0x00000000 */
huge   = 1.0e+300,
tiny   = 1.0e-300;

#ifdef __STDC__
	double __scalbn (double x, int n)
#else
	double __scalbn (x,n)
	double x; int n;
#endif
{
	int64_t ix;
	int64_t k;
	EXTRACT_WORDS64(ix,x);
	k = (ix >> 52) & 0x7ff;			/* extract exponent */
	if (k==0) {				/* 0 or subnormal x */
	    if ((ix & UINT64_C(0xfffffffffffff))==0) return x; /* +-0 */
	    x *= two54;
	    EXTRACT_WORDS64(ix,x);
	    k = ((ix >> 52) & 0x7ff) - 54;
	    }
	if (k==0x7ff) return x+x;		/* NaN or Inf */
	k = k+n;
	if (n> 50000 || k >  0x7fe)
	  return huge*__copysign(huge,x); /* overflow  */
	if (n< -50000) return tiny*__copysign(tiny,x); /*underflow*/
	if (k > 0)				/* normal result */
	    {INSERT_WORDS64(x,(ix&UINT64_C(0x800fffffffffffff))|(k<<52));
	      return x;}
	if (k <= -54)
	  return tiny*__copysign(tiny,x);	/*underflow*/
	k += 54;				/* subnormal result */
	INSERT_WORDS64(x,(ix&INT64_C(0x800fffffffffffff))|(k<<52));
	return x*twom54;
}
weak_alias (__scalbn, scalbn)
#ifdef NO_LONG_DOUBLE
strong_alias (__scalbn, __scalbnl)
weak_alias (__scalbn, scalbnl)
#endif
