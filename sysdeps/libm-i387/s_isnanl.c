/* s_isnanl.c -- long double version for i387 of s_isnan.c.
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
 * isnanl(x) returns 1 is x is nan, else 0;
 * no branching!
 */

#include "math.h"
#include "math_private.h"

#ifdef __STDC__
	int __isnanl(long double x)
#else
	int __isnanl(x)
	long double x;
#endif
{
	int32_t se,hx,lx;
	GET_LDOUBLE_WORDS(se,hx,lx,x);
	se = (se & 0x7fff) << 1;
	lx |= hx;
	/* The additional &hx is required because Intel's extended format
	   has the normally implicit 1 explicit present.  Sigh!  */
	se |= (u_int32_t)((lx|(-lx))&hx)>>31;
	se = 0xfffe - se;
	return (int)((u_int32_t)(se))>>16;
}
weak_alias (__isnanl, isnanl)
