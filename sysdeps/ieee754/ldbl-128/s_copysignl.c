/* s_copysignl.c -- long double version of s_copysign.c.
 * Conversion to long double by Jakub Jelinek, jj@ultra.linux.cz.
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
 * copysignl(long double x, long double y)
 * copysignl(x,y) returns a value with the magnitude of x and
 * with the sign bit of y.
 */

#include <math.h>
#include <math_private.h>

long double __copysignl(long double x, long double y)
{
	u_int64_t hx,hy;
	GET_LDOUBLE_MSW64(hx,x);
	GET_LDOUBLE_MSW64(hy,y);
	SET_LDOUBLE_MSW64(x,(hx&0x7fffffffffffffffULL)
			    |(hy&0x8000000000000000ULL));
        return x;
}
weak_alias (__copysignl, copysignl)
