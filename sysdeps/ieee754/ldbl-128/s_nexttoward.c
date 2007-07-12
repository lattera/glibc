/* s_nexttoward.c
 * Conversion from s_nextafter.c by Ulrich Drepper, Cygnus Support,
 * drepper@cygnus.com and Jakub Jelinek, jj@ultra.linux.cz.
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
 *	nexttoward(x,y)
 *	return the next machine floating-point number of x in the
 *	direction toward y.
 *   Special cases:
 */

#include "math.h"
#include <math_private.h>
#include <float.h>

#ifdef __STDC__
	double __nexttoward(double x, long double y)
#else
	double __nexttoward(x,y)
	double x;
	long double y;
#endif
{
	int32_t hx,ix;
	int64_t hy,iy;
	u_int32_t lx;
	u_int64_t ly;

	EXTRACT_WORDS(hx,lx,x);
	GET_LDOUBLE_WORDS64(hy,ly,y);
	ix = hx&0x7fffffff;		/* |x| */
	iy = hy&0x7fffffffffffffffLL;	/* |y| */

	if(((ix>=0x7ff00000)&&((ix-0x7ff00000)|lx)!=0) ||   /* x is nan */
	   ((iy>=0x7fff000000000000LL)&&((iy-0x7fff000000000000LL)|ly)!=0))
	   						    /* y is nan */
	   return x+y;
	if((long double) x==y) return y;	/* x=y, return y */
	if((ix|lx)==0) {			/* x == 0 */
	    double u;
	    INSERT_WORDS(x,(u_int32_t)((hy>>32)&0x80000000),1);/* return +-minsub */
	    u = math_opt_barrier (x);
	    u = u * u;
	    math_force_eval (u);		/* raise underflow flag */
	    return x;
	}
	if(hx>=0) {				/* x > 0 */
	    if (hy<0||(ix>>20)>(iy>>48)-0x3c00
		|| ((ix>>20)==(iy>>48)-0x3c00
		    && (((((int64_t)hx)<<28)|(lx>>4))>(hy&0x0000ffffffffffffLL)
			|| (((((int64_t)hx)<<28)|(lx>>4))==(hy&0x0000ffffffffffffLL)
			    && (lx&0xf)>(ly>>60))))) {	/* x > y, x -= ulp */
		if(lx==0) hx -= 1;
		lx -= 1;
	    } else {				/* x < y, x += ulp */
		lx += 1;
		if(lx==0) hx += 1;
	    }
	} else {				/* x < 0 */
	    if (hy>=0||(ix>>20)>(iy>>48)-0x3c00
		|| ((ix>>20)==(iy>>48)-0x3c00
		    && (((((int64_t)hx)<<28)|(lx>>4))>(hy&0x0000ffffffffffffLL)
			|| (((((int64_t)hx)<<28)|(lx>>4))==(hy&0x0000ffffffffffffLL)
			    && (lx&0xf)>(ly>>60))))) {	/* x < y, x -= ulp */
		if(lx==0) hx -= 1;
		lx -= 1;
	    } else {				/* x > y, x += ulp */
		lx += 1;
		if(lx==0) hx += 1;
	    }
	}
	hy = hx&0x7ff00000;
	if(hy>=0x7ff00000) {
	  x = x+x;	/* overflow  */
	  if (FLT_EVAL_METHOD != 0 && FLT_EVAL_METHOD != 1)
	    /* Force conversion to double.  */
	    asm ("" : "+m"(x));
	  return x;
	}
	if(hy<0x00100000) {
	    double u = x*x;			/* underflow */
	    math_force_eval (u);		/* raise underflow flag */
	}
	INSERT_WORDS(x,hx,lx);
	return x;
}
weak_alias (__nexttoward, nexttoward)
