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

/* __ieee754_log2(x)
 * Return the logarithm to base 2 of x
 *
 * Method :
 *   1. Argument Reduction: find k and f such that
 *			x = 2^k * (1+f),
 *	   where  sqrt(2)/2 < 1+f < sqrt(2) .
 *
 *   2. Approximation of log(1+f).
 *	Let s = f/(2+f) ; based on log(1+f) = log(1+s) - log(1-s)
 *		 = 2s + 2/3 s**3 + 2/5 s**5 + .....,
 *		 = 2s + s*R
 *      We use a special Reme algorithm on [0,0.1716] to generate
 *	a polynomial of degree 14 to approximate R The maximum error
 *	of this polynomial approximation is bounded by 2**-58.45. In
 *	other words,
 *			2      4      6      8      10      12      14
 *	    R(z) ~ Lg1*s +Lg2*s +Lg3*s +Lg4*s +Lg5*s  +Lg6*s  +Lg7*s
 *	(the values of Lg1 to Lg7 are listed in the program)
 *	and
 *	    |      2          14          |     -58.45
 *	    | Lg1*s +...+Lg7*s    -  R(z) | <= 2
 *	    |                             |
 *	Note that 2s = f - s*f = f - hfsq + s*hfsq, where hfsq = f*f/2.
 *	In order to guarantee error in log below 1ulp, we compute log
 *	by
 *		log(1+f) = f - s*(f - R)	(if f is not too large)
 *		log(1+f) = f - (hfsq - s*(hfsq+R)).	(better accuracy)
 *
 *	3. Finally,  log(x) = k + log(1+f).
 *			    = k+(f-(hfsq-(s*(hfsq+R))))
 *
 * Special cases:
 *	log2(x) is NaN with signal if x < 0 (including -INF) ;
 *	log2(+INF) is +INF; log(0) is -INF with signal;
 *	log2(NaN) is that NaN with no signal.
 *
 * Constants:
 * The hexadecimal values are the intended ones for the following
 * constants. The decimal values may be used, provided that the
 * compiler will convert from decimal to binary accurately enough
 * to produce the hexadecimal values shown.
 */

#include <math.h>
#include <math_private.h>

static const double ln2 = 0.69314718055994530942;
static const double two54 = 1.80143985094819840000e+16;	/* 4350000000000000 */
static const double Lg1 = 6.666666666666735130e-01;	/* 3FE5555555555593 */
static const double Lg2 = 3.999999999940941908e-01;	/* 3FD999999997FA04 */
static const double Lg3 = 2.857142874366239149e-01;	/* 3FD2492494229359 */
static const double Lg4 = 2.222219843214978396e-01;	/* 3FCC71C51D8E78AF */
static const double Lg5 = 1.818357216161805012e-01;	/* 3FC7466496CB03DE */
static const double Lg6 = 1.531383769920937332e-01;	/* 3FC39A09D078C69F */
static const double Lg7 = 1.479819860511658591e-01;	/* 3FC2F112DF3E5244 */

static const double zero = 0.0;

double
__ieee754_log2 (double x)
{
  double hfsq, f, s, z, R, w, t1, t2, dk;
  int64_t hx, i, j;
  int32_t k;

  EXTRACT_WORDS64 (hx, x);

  k = 0;
  if (hx < INT64_C(0x0010000000000000))
    {				/* x < 2**-1022  */
      if (__builtin_expect ((hx & UINT64_C(0x7fffffffffffffff)) == 0, 0))
	return -two54 / (x - x);	/* log(+-0)=-inf */
      if (__builtin_expect (hx < 0, 0))
	return (x - x) / (x - x);	/* log(-#) = NaN */
      k -= 54;
      x *= two54;		/* subnormal number, scale up x */
      EXTRACT_WORDS64 (hx, x);
    }
  if (__builtin_expect (hx >= UINT64_C(0x7ff0000000000000), 0))
    return x + x;
  k += (hx >> 52) - 1023;
  hx &= UINT64_C(0x000fffffffffffff);
  i = (hx + UINT64_C(0x95f6400000000)) & UINT64_C(0x10000000000000);
  /* normalize x or x/2 */
  INSERT_WORDS64 (x, hx | (i ^ UINT64_C(0x3ff0000000000000)));
  k += (i >> 52);
  dk = (double) k;
  f = x - 1.0;
  if ((UINT64_C(0x000fffffffffffff) & (2 + hx)) < 3)
    {				/* |f| < 2**-20 */
      if (f == zero)
	return dk;
      R = f * f * (0.5 - 0.33333333333333333 * f);
      return dk - (R - f) / ln2;
    }
  s = f / (2.0 + f);
  z = s * s;
  i = hx - UINT64_C(0x6147a00000000);
  w = z * z;
  j = UINT64_C(0x6b85100000000) - hx;
  t1 = w * (Lg2 + w * (Lg4 + w * Lg6));
  t2 = z * (Lg1 + w * (Lg3 + w * (Lg5 + w * Lg7)));
  i |= j;
  R = t2 + t1;
  if (i > 0)
    {
      hfsq = 0.5 * f * f;
      return dk - ((hfsq - (s * (hfsq + R))) - f) / ln2;
    }
  else
    {
      return dk - ((s * (f - R)) - f) / ln2;
    }
}

strong_alias (__ieee754_log2, __log2_finite)
