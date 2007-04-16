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

#ifdef __STDC__
	long double __nextafterl(long double x, long double y)
#else
	long double __nextafterl(x,y)
	long double x,y;
#endif
{
	int64_t hx,hy,ihx,ihy,ilx,ily;
	u_int64_t lx,ly;

	GET_LDOUBLE_WORDS64(hx,lx,x);
	GET_LDOUBLE_WORDS64(hy,ly,y);
	ihx = hx&0x7fffffffffffffffLL;		/* |hx| */
	ilx = lx&0x7fffffffffffffffLL;		/* |lx| */
	ihy = hy&0x7fffffffffffffffLL;		/* |hy| */
	ily = ly&0x7fffffffffffffffLL;		/* |ly| */

	if((((ihx&0x7ff0000000000000LL)==0x7ff0000000000000LL)&&
	    ((ihx&0x000fffffffffffffLL)!=0)) ||   /* x is nan */
	   (((ihy&0x7ff0000000000000LL)==0x7ff0000000000000LL)&&
	    ((ihy&0x000fffffffffffffLL)!=0)))     /* y is nan */
	    return x+y; /* signal the nan */
	if(x==y)
	    return y;		/* x=y, return y */
	if(ihx == 0 && ilx == 0) {			/* x == 0 */
	    long double u;
	    SET_LDOUBLE_WORDS64(x,hy&0x8000000000000000ULL,1);/* return +-minsubnormal */
	    u = math_opt_barrier (u);
	    u = u * u;
	    math_force_eval (u);		/* raise underflow flag */
	    return x;
	}
	if(ihx>=0) {			/* x > 0 */
	    if(ihx>ihy||((ihx==ihy)&&(ilx>ily))) {	/* x > y, x -= ulp */

	        if(ilx==0)
		    hx--;
		else
		    lx--;
	    } else {				/* x < y, x += ulp */
	        if((hx==0x7fefffffffffffffLL)&&(lx==0x7c8ffffffffffffeLL))
		  {
		    SET_LDOUBLE_WORDS64(x,0x7ff0000000000000,0x8000000000000000);
		    return x;
		  }
	        else if((hx==0xffefffffffffffffLL)&&(lx==0xfc8ffffffffffffeLL))
		  {
		    SET_LDOUBLE_WORDS64(x,0xfff0000000000000,0x8000000000000000);
		    return x;
		  }
		else if((lx&0x7fffffffffffffff)==0) hx++;
		else
		  lx++;
	    }
	} else {				/* x < 0 */
	    if(ihy>=0||ihx>ihy||((ihx==ihy)&&(ilx>ily))){/* x < y, x -= ulp */
		if((lx&0x7fffffffffffffff)==0)
		    hx--;
		else
		    lx--;
	    } else {				/* x > y, x += ulp */
		if((lx&0x7fffffffffffffff)==0) hx++;
		else
		  lx++;
	    }
	}
	hy = hx&0x7ff0000000000000LL;
	if(hy==0x7ff0000000000000LL) return x+x;/* overflow  */
	if(hy==0) {
	    long double u = x * x;		/* underflow */
	    math_force_eval (u);		/* raise underflow flag */
	}
	SET_LDOUBLE_WORDS64(x,hx,lx);
	return x;
}
strong_alias (__nextafterl, __nexttowardl)
long_double_symbol (libm, __nextafterl, nextafterl);
long_double_symbol (libm, __nexttowardl, nexttowardl);
