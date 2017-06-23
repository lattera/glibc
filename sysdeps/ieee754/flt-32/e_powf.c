/* e_powf.c -- float version of e_pow.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */
/* Copyright (C) 2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

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

#include <math.h>
#include <math_private.h>

static const float huge = 1.0e+30, tiny = 1.0e-30;

static const float
bp[] = {1.0, 1.5,},
zero    =  0.0,
one	=  1.0,
two	=  2.0,
two24	=  16777216.0,	/* 0x4b800000 */
	/* poly coefs for (3/2)*(log(x)-2s-2/3*s**3 */
L1  =  6.0000002384e-01, /* 0x3f19999a */
L2  =  4.2857143283e-01, /* 0x3edb6db7 */
L3  =  3.3333334327e-01, /* 0x3eaaaaab */
L4  =  2.7272811532e-01, /* 0x3e8ba305 */
L5  =  2.3066075146e-01, /* 0x3e6c3255 */
L6  =  2.0697501302e-01, /* 0x3e53f142 */
P1   =  1.6666667163e-01, /* 0x3e2aaaab */
P2   = -2.7777778450e-03, /* 0xbb360b61 */
P3   =  6.6137559770e-05, /* 0x388ab355 */
P4   = -1.6533901999e-06, /* 0xb5ddea0e */
P5   =  4.1381369442e-08, /* 0x3331bb4c */
ovt =  4.2995665694e-08; /* -(128-log2(ovfl+.5ulp)) */

static const double
	dp[] = { 0.0, 0x1.2b803473f7ad1p-1, }, /* log2(1.5) */
	lg2 = M_LN2,
	cp = 2.0/3.0/M_LN2,
	invln2 = 1.0/M_LN2;

float
__ieee754_powf(float x, float y)
{
	float z, ax, s;
	double d1, d2;
	int32_t i,j,k,yisint,n;
	int32_t hx,hy,ix,iy;

	GET_FLOAT_WORD(hy,y);
	iy = hy&0x7fffffff;

    /* y==zero: x**0 = 1 */
	if(iy==0 && !issignaling (x)) return one;

    /* x==+-1 */
	if(x == 1.0 && !issignaling (y)) return one;
	if(x == -1.0 && isinf(y)) return one;

	GET_FLOAT_WORD(hx,x);
	ix = hx&0x7fffffff;

    /* +-NaN return x+y */
	if(__builtin_expect(ix > 0x7f800000 ||
			    iy > 0x7f800000, 0))
		return x+y;

    /* special value of y */
	if (__builtin_expect(iy==0x7f800000, 0)) {	/* y is +-inf */
	    if (ix==0x3f800000)
		return  y - y;	/* inf**+-1 is NaN */
	    else if (ix > 0x3f800000)/* (|x|>1)**+-inf = inf,0 */
		return (hy>=0)? y: zero;
	    else			/* (|x|<1)**-,+inf = inf,0 */
		return (hy<0)?-y: zero;
	}
	if(iy==0x3f800000) {	/* y is  +-1 */
	    if(hy<0) return one/x; else return x;
	}
	if(hy==0x40000000) return x*x; /* y is  2 */
	if(hy==0x3f000000) {	/* y is  0.5 */
	    if(__builtin_expect(hx>=0, 1))	/* x >= +0 */
	    return __ieee754_sqrtf(x);
	}

    /* determine if y is an odd int when x < 0
     * yisint = 0	... y is not an integer
     * yisint = 1	... y is an odd int
     * yisint = 2	... y is an even int
     */
	yisint  = 0;
	if(hx<0) {
	    if(iy>=0x4b800000) yisint = 2; /* even integer y */
	    else if(iy>=0x3f800000) {
		k = (iy>>23)-0x7f;	   /* exponent */
		j = iy>>(23-k);
		if((j<<(23-k))==iy) yisint = 2-(j&1);
	    }
	}

	ax   = fabsf(x);
    /* special value of x */
	if(__builtin_expect(ix==0x7f800000||ix==0||ix==0x3f800000, 0)){
	    z = ax;			/*x is +-0,+-inf,+-1*/
	    if(hy<0) z = one/z;	/* z = (1/|x|) */
	    if(hx<0) {
		if(((ix-0x3f800000)|yisint)==0) {
		    z = (z-z)/(z-z); /* (-1)**non-int is NaN */
		} else if(yisint==1)
		    z = -z;		/* (x<0)**odd = -(|x|**odd) */
	    }
	    return z;
	}

    /* (x<0)**(non-int) is NaN */
	if(__builtin_expect(((((u_int32_t)hx>>31)-1)|yisint)==0, 0))
	    return (x-x)/(x-x);

    /* |y| is huge */
	if(__builtin_expect(iy>0x4d000000, 0)) { /* if |y| > 2**27 */
	/* over/underflow if x is not close to one */
	    if(ix<0x3f7ffff8) return (hy<0)? huge*huge:tiny*tiny;
	    if(ix>0x3f800007) return (hy>0)? huge*huge:tiny*tiny;
	/* now |1-x| is tiny <= 2**-20, suffice to compute
	   log(x) by x-x^2/2+x^3/3-x^4/4 */
	    d2 = ax-1;		/* d2 has 20 trailing zeros.  */
	    d2 = d2 * invln2 -
		 (d2 * d2) * (0.5 - d2 * (0.333333333333 - d2 * 0.25)) * invln2;
	} else {
	    /* Avoid internal underflow for tiny y.  The exact value
	       of y does not matter if |y| <= 2**-32.  */
	    if (iy < 0x2f800000)
	      SET_FLOAT_WORD (y, (hy & 0x80000000) | 0x2f800000);
	    n = 0;
	/* take care subnormal number */
	    if(ix<0x00800000)
		{ax *= two24; n -= 24; GET_FLOAT_WORD(ix,ax); }
	    n  += ((ix)>>23)-0x7f;
	    j  = ix&0x007fffff;
	/* determine interval */
	    ix = j|0x3f800000;		/* normalize ix */
	    if(j<=0x1cc471) k=0;	/* |x|<sqrt(3/2) */
	    else if(j<0x5db3d7) k=1;	/* |x|<sqrt(3)   */
	    else {k=0;n+=1;ix -= 0x00800000;}
	    SET_FLOAT_WORD(ax,ix);

	/* compute d1 = (x-1)/(x+1) or (x-1.5)/(x+1.5) */
	    d1 = (ax-(double)bp[k])/(ax+(double)bp[k]);
	/* compute d2 = log(ax) */
	    d2 = d1 * d1;
	    d2 = 3.0 + d2 + d2*d2*(L1+d2*(L2+d2*(L3+d2*(L4+d2*(L5+d2*L6)))));
	/* 2/(3log2)*(d2+...) */
	    d2 = d1*d2*cp;
	/* log2(ax) = (d2+..)*2/(3*log2) */
	    d2 = d2+dp[k]+(double)n;
	}

	s = one; /* s (sign of result -ve**odd) = -1 else = 1 */
	if(((((u_int32_t)hx>>31)-1)|(yisint-1))==0)
	    s = -one;	/* (-ve)**(odd int) */

    /* compute y * d2 */
	d1 = y * d2;
	z = d1;
	GET_FLOAT_WORD(j,z);
	if (__builtin_expect(j>0x43000000, 0))		/* if z > 128 */
	    return s*huge*huge;				/* overflow */
	else if (__builtin_expect(j==0x43000000, 0)) {	/* if z == 128 */
	    if(ovt>(z-d1)) return s*huge*huge;	/* overflow */
	}
	else if (__builtin_expect((j&0x7fffffff)>0x43160000, 0))/* z <= -150 */
	    return s*tiny*tiny;				/* underflow */
	else if (__builtin_expect((u_int32_t) j==0xc3160000, 0)){/* z == -150*/
	    if(0.0<=(z-d1)) return s*tiny*tiny;		/* underflow */
	}
    /*
     * compute 2**d1
     */
	i = j&0x7fffffff;
	k = (i>>23)-0x7f;
	n = 0;
	if(i>0x3f000000) {		/* if |z| > 0.5, set n = [z+0.5] */
	    n = j+(0x00800000>>(k+1));
	    k = ((n&0x7fffffff)>>23)-0x7f;	/* new k for n */
	    SET_FLOAT_WORD(z,n&~(0x007fffff>>k));
	    n = ((n&0x007fffff)|0x00800000)>>(23-k);
	    if(j<0) n = -n;
	    d1 -= z;
	}
	d1 = d1 * lg2;
	d2 = d1*d1;
	d2 = d1 - d2*(P1+d2*(P2+d2*(P3+d2*(P4+d2*P5))));
	d2 = (d1*d2)/(d2-two);
	z = one - (d2-d1);
	GET_FLOAT_WORD(j,z);
	j += (n<<23);
	if((j>>23)<=0)	/* subnormal output */
	  {
	    z = __scalbnf (z, n);
	    float force_underflow = z * z;
	    math_force_eval (force_underflow);
	  }
	else SET_FLOAT_WORD(z,j);
	return s*z;
}
strong_alias (__ieee754_powf, __powf_finite)
