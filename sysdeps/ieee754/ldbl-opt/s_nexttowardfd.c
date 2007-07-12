/* Single precision version of nexttoward.c.
   Conversion to IEEE single float by Jakub Jelinek, jj@ultra.linux.cz. */
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

/* IEEE functions
 *	__nexttowardfd(x,y)
 *	return the next machine floating-point number of x in the
 *	direction toward y.
 * This is for machines which use different binary type for double and
 * long double conditionally, y is long double equal to double.
 *   Special cases:
 */

#include <math.h>
#include <math_private.h>
#include <math_ldbl_opt.h>
#include <float.h>

float __nldbl_nexttowardf(float x, double y);

float __nldbl_nexttowardf(float x, double y)
{
	int32_t hx,hy,ix,iy;
	u_int32_t ly;

	GET_FLOAT_WORD(hx,x);
	EXTRACT_WORDS(hy,ly,y);
	ix = hx&0x7fffffff;		/* |x| */
	iy = hy&0x7fffffff;		/* |y| */

	if((ix>0x7f800000) ||				   /* x is nan */
	   ((iy>=0x7ff00000)&&((iy-0x7ff00000)|ly)!=0))    /* y is nan */
	   return x+y;
	if((double) x==y) return y;		/* x=y, return y */
	if(ix==0) {				/* x == 0 */
	    float u;
	    SET_FLOAT_WORD(x,(u_int32_t)(hy&0x80000000)|1);/* return +-minsub*/
	    u = math_opt_barrier (x);
	    u = u * u;
	    math_force_eval (u);		/* raise underflow flag */
	    return x;
	}
	if(hx>=0) {				/* x > 0 */
	    if(hy<0||(ix>>23)>(iy>>20)-0x380
	       || ((ix>>23)==(iy>>20)-0x380
		   && (ix&0x7fffff)>(((hy<<3)|(ly>>29))&0x7fffff)))	/* x > y, x -= ulp */
		hx -= 1;
	    else				/* x < y, x += ulp */
		hx += 1;
	} else {				/* x < 0 */
	    if(hy>=0||(ix>>23)>(iy>>20)-0x380
	       || ((ix>>23)==(iy>>20)-0x380
		   && (ix&0x7fffff)>(((hy<<3)|(ly>>29))&0x7fffff)))	/* x < y, x -= ulp */
		hx -= 1;
	    else				/* x > y, x += ulp */
		hx += 1;
	}
	hy = hx&0x7f800000;
	if(hy>=0x7f800000) {
	  x = x+x;	/* overflow  */
	  if (FLT_EVAL_METHOD != 0)
	    /* Force conversion to float.  */
	    asm ("" : "+m"(x));
	  return x;
	}
	if(hy<0x00800000) {
	    float u = x*x;			/* underflow */
	    math_force_eval (u);		/* raise underflow flag */
	}
	SET_FLOAT_WORD(x,hx);
	return x;
}

#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __nldbl_nexttowardf, nexttowardf, GLIBC_2_1);
#endif
