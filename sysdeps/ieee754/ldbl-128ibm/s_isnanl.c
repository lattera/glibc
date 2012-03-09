/* s_isnanl.c -- long double version of s_isnan.c.
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
 * isnanl(x) returns 1 is x is nan, else 0;
 * no branching!
 */

#include <math.h>
#include <math_private.h>
#include <math_ldbl_opt.h>

int
___isnanl (long double x)
{
	int64_t hx,lx;
	GET_LDOUBLE_WORDS64(hx,lx,x);
	hx &= 0x7fffffffffffffffLL;
	hx = 0x7ff0000000000000LL - hx;
	return (int)((u_int64_t)hx>>63);
}
hidden_ver (___isnanl, __isnanl)
#ifndef IS_IN_libm
weak_alias (___isnanl, ____isnanl)
long_double_symbol (libc, ___isnanl, isnanl);
long_double_symbol (libc, ____isnanl, __isnanl);
#endif
