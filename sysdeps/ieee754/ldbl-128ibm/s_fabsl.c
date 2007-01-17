/* s_fabsl.c -- long double version of s_fabs.c.
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

#if defined(LIBM_SCCS) && !defined(lint)
static char rcsid[] = "$NetBSD: $";
#endif


/*
 * fabsl(x) returns the absolute value of x.
 */

#include "math.h"
#include "math_private.h"
#include <math_ldbl_opt.h>

#ifdef __STDC__
	long double __fabsl(long double x)
#else
	long double __fabsl(x)
	long double x;
#endif
{
	u_int64_t hx, lx;
	GET_LDOUBLE_WORDS64(hx,lx,x);
	lx = lx ^ ( hx & 0x8000000000000000LL );
	hx = hx & 0x7fffffffffffffffLL;
	SET_LDOUBLE_WORDS64(x,hx,lx);
	return x;
}
long_double_symbol (libm, __fabsl, fabsl);
