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

#include <math.h>
#include <math_private.h>
#include <math_ldbl_opt.h>

long double __nextafterl(long double x, long double y)
{
	int64_t hx,hy,ihx,ihy,ilx;
	u_int64_t lx;
	u_int64_t ly __attribute__ ((unused));

	GET_LDOUBLE_WORDS64(hx,lx,x);
	GET_LDOUBLE_WORDS64(hy,ly,y);
	ihx = hx&0x7fffffffffffffffLL;		/* |hx| */
	ilx = lx&0x7fffffffffffffffLL;		/* |lx| */
	ihy = hy&0x7fffffffffffffffLL;		/* |hy| */

	if((((ihx&0x7ff0000000000000LL)==0x7ff0000000000000LL)&&
	    ((ihx&0x000fffffffffffffLL)!=0)) ||   /* x is nan */
	   (((ihy&0x7ff0000000000000LL)==0x7ff0000000000000LL)&&
	    ((ihy&0x000fffffffffffffLL)!=0)))     /* y is nan */
	    return x+y; /* signal the nan */
	if(x==y)
	    return y;		/* x=y, return y */
	if(ihx == 0 && ilx == 0) {			/* x == 0 */
	    long double u;
	    hy = (hy & 0x8000000000000000ULL) | 1;
	    SET_LDOUBLE_WORDS64(x,hy,0ULL);/* return +-minsubnormal */
	    u = math_opt_barrier (x);
	    u = u * u;
	    math_force_eval (u);		/* raise underflow flag */
	    return x;
	}
	
	long double u;
	if(x > y) {	/* x > y, x -= ulp */
	    if((hx==0xffefffffffffffffLL)&&(lx==0xfc8ffffffffffffeLL))
	      return x+x;	/* overflow, return -inf */
	    if (hx >= 0x7ff0000000000000LL) {
	      SET_LDOUBLE_WORDS64(u,0x7fefffffffffffffLL,0x7c8ffffffffffffeLL);
	      return u;
	    }
	    if(ihx <= 0x0360000000000000LL) {  /* x <= LDBL_MIN */
	      u = math_opt_barrier (x);
	      x -= __LDBL_DENORM_MIN__;
	      if (ihx < 0x0360000000000000LL
		  || (hx > 0 && (int64_t) lx <= 0)
		  || (hx < 0 && (int64_t) lx > 1)) {
		u = u * u;
		math_force_eval (u);		/* raise underflow flag */
	      }
	      return x;
	    }
	    if (ihx < 0x06a0000000000000LL) { /* ulp will denormal */
	      SET_LDOUBLE_WORDS64(u,(hx&0x7ff0000000000000LL),0ULL);
	      u *= 0x1.0000000000000p-105L;
	    } else
	      SET_LDOUBLE_WORDS64(u,(hx&0x7ff0000000000000LL)-0x0690000000000000LL,0ULL);
	    return x - u;
	} else {				/* x < y, x += ulp */
	    if((hx==0x7fefffffffffffffLL)&&(lx==0x7c8ffffffffffffeLL))
	      return x+x;	/* overflow, return +inf */
	    if ((u_int64_t) hx >= 0xfff0000000000000ULL) {
	      SET_LDOUBLE_WORDS64(u,0xffefffffffffffffLL,0xfc8ffffffffffffeLL);
	      return u;
	    }
	    if(ihx <= 0x0360000000000000LL) {  /* x <= LDBL_MIN */
	      u = math_opt_barrier (x);
	      x += __LDBL_DENORM_MIN__;
	      if (ihx < 0x0360000000000000LL
		  || (hx > 0 && (int64_t) lx < 0 && lx != 0x8000000000000001LL)
		  || (hx < 0 && (int64_t) lx >= 0)) {
		u = u * u;
		math_force_eval (u);		/* raise underflow flag */
	      }
	      if (x == 0.0L)	/* handle negative __LDBL_DENORM_MIN__ case */
		x = -0.0L;
	      return x;
	    }
	    if (ihx < 0x06a0000000000000LL) { /* ulp will denormal */
	      SET_LDOUBLE_WORDS64(u,(hx&0x7ff0000000000000LL),0ULL);
	      u *= 0x1.0000000000000p-105L;
	    } else
	      SET_LDOUBLE_WORDS64(u,(hx&0x7ff0000000000000LL)-0x0690000000000000LL,0ULL);
	    return x + u;
	}
}
strong_alias (__nextafterl, __nexttowardl)
long_double_symbol (libm, __nextafterl, nextafterl);
long_double_symbol (libm, __nexttowardl, nexttowardl);
