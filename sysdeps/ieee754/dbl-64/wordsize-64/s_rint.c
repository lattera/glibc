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
 * rint(x)
 * Return x rounded to integral value according to the prevailing
 * rounding mode.
 * Method:
 *	Using floating addition.
 * Exception:
 *	Inexact flag raised if x not equal to rint(x).
 */

#include "math.h"
#include "math_private.h"

static const double
TWO52[2]={
  4.50359962737049600000e+15, /* 0x43300000, 0x00000000 */
 -4.50359962737049600000e+15, /* 0xC3300000, 0x00000000 */
};

double
__rint(double x)
{
	int64_t i0,sx;
	int32_t j0;
	EXTRACT_WORDS64(i0,x);
	sx = (i0>>63)&1;
	j0 = ((i0>>52)&0x7ff)-0x3ff;
	if(j0<52) {
	    if(j0<0) {
		if((i0 & UINT64_C(0x7fffffffffffffff))==0) return x;
		uint64_t i = i0 & UINT64_C(0xfffffffffffff);
		i0 &= UINT64_C(0xfffe000000000000);
		i0 |= (((i|-i) >> 12) & UINT64_C(0x8000000000000));
		INSERT_WORDS64(x,i0);
		double w = TWO52[sx]+x;
		double t =  w-TWO52[sx];
		EXTRACT_WORDS64(i0,t);
		INSERT_WORDS64(t,(i0&UINT64_C(0x7fffffffffffffff))|(sx<<63));
		return t;
	    } else {
		uint64_t i = UINT64_C(0x000fffffffffffff)>>j0;
		if((i0&i)==0) return x; /* x is integral */
		i>>=1;
		if((i0&i)!=0)
		    i0 = (i0&(~i))|(UINT64_C(0x4000000000000)>>j0);
	    }
	} else {
	    if(j0==0x400) return x+x;	/* inf or NaN */
	    else return x;		/* x is integral */
	}
	INSERT_WORDS64(x,i0);
	double w = TWO52[sx]+x;
	return w-TWO52[sx];
}
#ifndef __rint
weak_alias (__rint, rint)
# ifdef NO_LONG_DOUBLE
strong_alias (__rint, __rintl)
weak_alias (__rint, rintl)
# endif
#endif
