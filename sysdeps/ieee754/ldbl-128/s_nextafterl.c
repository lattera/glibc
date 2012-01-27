/* s_nextafterl.c -- long double version of s_nextafter.c.
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

/* IEEE functions
 *	nextafterl(x,y)
 *	return the next machine floating-point number of x in the
 *	direction toward y.
 *   Special cases:
 */

#include "math.h"
#include <math_private.h>

long double __nextafterl(long double x, long double y)
{
	int64_t hx,hy,ix,iy;
	u_int64_t lx,ly;

	GET_LDOUBLE_WORDS64(hx,lx,x);
	GET_LDOUBLE_WORDS64(hy,ly,y);
	ix = hx&0x7fffffffffffffffLL;		/* |x| */
	iy = hy&0x7fffffffffffffffLL;		/* |y| */

	if(((ix>=0x7fff000000000000LL)&&((ix-0x7fff000000000000LL)|lx)!=0) ||   /* x is nan */
	   ((iy>=0x7fff000000000000LL)&&((iy-0x7fff000000000000LL)|ly)!=0))     /* y is nan */
	   return x+y;
	if(x==y) return y;		/* x=y, return y */
	if((ix|lx)==0) {			/* x == 0 */
	    long double u;
	    SET_LDOUBLE_WORDS64(x,hy&0x8000000000000000ULL,1);/* return +-minsubnormal */
	    u = math_opt_barrier (x);
	    u = u * u;
	    math_force_eval (u);		/* raise underflow flag */
	    return x;
	}
	if(hx>=0) {			/* x > 0 */
	    if(hx>hy||((hx==hy)&&(lx>ly))) {	/* x > y, x -= ulp */
		if(lx==0) hx--;
		lx--;
	    } else {				/* x < y, x += ulp */
		lx++;
		if(lx==0) hx++;
	    }
	} else {				/* x < 0 */
	    if(hy>=0||hx>hy||((hx==hy)&&(lx>ly))){/* x < y, x -= ulp */
		if(lx==0) hx--;
		lx--;
	    } else {				/* x > y, x += ulp */
		lx++;
		if(lx==0) hx++;
	    }
	}
	hy = hx&0x7fff000000000000LL;
	if(hy==0x7fff000000000000LL) return x+x;/* overflow  */
	if(hy==0) {
	    long double u = x*x;		/* underflow */
	    math_force_eval (u);		/* raise underflow flag */
	}
	SET_LDOUBLE_WORDS64(x,hx,lx);
	return x;
}
weak_alias (__nextafterl, nextafterl)
strong_alias (__nextafterl, __nexttowardl)
weak_alias (__nextafterl, nexttowardl)
