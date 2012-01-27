/* s_frexpl.c -- long double version of s_frexp.c.
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
 * for non-zero x
 *	x = frexpl(arg,&exp);
 * return a long double fp quantity x such that 0.5 <= |x| <1.0
 * and the corresponding binary exponent "exp". That is
 *	arg = x*2^exp.
 * If arg is inf, 0.0, or NaN, then frexpl(arg,&exp) returns arg
 * with *exp=0.
 */

#include "math.h"
#include "math_private.h"
#include <math_ldbl_opt.h>

static const long double
two107 = 162259276829213363391578010288128.0; /* 0x4670000000000000, 0 */

long double __frexpl(long double x, int *eptr)
{
	u_int64_t hx, lx, ix, ixl;
	int64_t explo;
	GET_LDOUBLE_WORDS64(hx,lx,x);
	ixl = 0x7fffffffffffffffULL&lx;
	ix =  0x7fffffffffffffffULL&hx;
	*eptr = 0;
	if(ix>=0x7ff0000000000000ULL||((ix|ixl)==0)) return x;	/* 0,inf,nan */
	if (ix<0x0010000000000000ULL) {		/* subnormal */
	    x *= two107;
	    GET_LDOUBLE_MSW64(hx,x);
	    ix = hx&0x7fffffffffffffffULL;
	    *eptr = -107;
	}
	*eptr += (ix>>52)-1022;

	if (ixl != 0ULL) {
	  explo = (ixl>>52) - (ix>>52) + 0x3fe;
	  if ((ixl&0x7ff0000000000000ULL) == 0LL) {
	    /* the lower double is a denomal so we need to correct its
	       mantissa and perhaps its exponent.  */
	    int cnt;

	    if (sizeof (ixl) == sizeof (long))
	      cnt = __builtin_clzl (ixl);
	    else if ((ixl >> 32) != 0)
	      cnt = __builtin_clzl ((long) (ixl >> 32));
	    else
	      cnt = __builtin_clzl ((long) ixl) + 32;
	    cnt = cnt - 12;
	    lx = (lx&0x8000000000000000ULL) | ((explo-cnt)<<52)
	       | ((ixl<<(cnt+1))&0x000fffffffffffffULL);
	  } else
	    lx = (lx&0x800fffffffffffffULL) | (explo<<52);
	} else
	  lx = 0ULL;

	hx = (hx&0x800fffffffffffffULL) | 0x3fe0000000000000ULL;
	SET_LDOUBLE_WORDS64(x,hx,lx);
	return x;
}
#ifdef IS_IN_libm
long_double_symbol (libm, __frexpl, frexpl);
#else
long_double_symbol (libc, __frexpl, frexpl);
#endif
