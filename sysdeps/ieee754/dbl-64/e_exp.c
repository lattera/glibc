/* EXP function - Compute double precision exponential */
/*
 * IBM Accurate Mathematical Library
 * written by International Business Machines Corp.
 * Copyright (C) 2001-2017 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
/***************************************************************************/
/*  MODULE_NAME:uexp.c                                                     */
/*                                                                         */
/*  FUNCTION:uexp                                                          */
/*           exp1                                                          */
/*                                                                         */
/* FILES NEEDED:dla.h endian.h mpa.h mydefs.h uexp.h                       */
/*              mpa.c mpexp.x                                              */
/*                                                                         */
/* An ultimate exp routine. Given an IEEE double machine number x          */
/* it computes the correctly rounded (to nearest) value of e^x             */
/* Assumption: Machine arithmetic operations are performed in              */
/* round to nearest mode of IEEE 754 standard.                             */
/*                                                                         */
/***************************************************************************/

/*  IBM exp(x) replaced by following exp(x) in 2017. IBM exp1(x,xx) remains.  */
/* exp(x)
   Hybrid algorithm of Peter Tang's Table driven method (for large
   arguments) and an accurate table (for small arguments).
   Written by K.C. Ng, November 1988.
   Revised by Patrick McGehearty, Nov 2017 to use j/64 instead of j/32
   Method (large arguments):
	1. Argument Reduction: given the input x, find r and integer k
	   and j such that
	             x = (k+j/64)*(ln2) + r,  |r| <= (1/128)*ln2

	2. exp(x) = 2^k * (2^(j/64) + 2^(j/64)*expm1(r))
	   a. expm1(r) is approximated by a polynomial:
	      expm1(r) ~ r + t1*r^2 + t2*r^3 + ... + t5*r^6
	      Here t1 = 1/2 exactly.
	   b. 2^(j/64) is represented to twice double precision
	      as TBL[2j]+TBL[2j+1].

   Note: If divide were fast enough, we could use another approximation
	 in 2.a:
	      expm1(r) ~ (2r)/(2-R), R = r - r^2*(t1 + t2*r^2)
	      (for the same t1 and t2 as above)

   Special cases:
	exp(INF) is INF, exp(NaN) is NaN;
	exp(-INF)=  0;
	for finite argument, only exp(0)=1 is exact.

   Accuracy:
	According to an error analysis, the error is always less than
	an ulp (unit in the last place).  The largest errors observed
	are less than 0.55 ulp for normal results and less than 0.75 ulp
	for subnormal results.

   Misc. info.
	For IEEE double
		if x >  7.09782712893383973096e+02 then exp(x) overflow
		if x < -7.45133219101941108420e+02 then exp(x) underflow.  */

#include <math.h>
#include <math-svid-compat.h>
#include <math_private.h>
#include <errno.h>
#include "endian.h"
#include "uexp.h"
#include "uexp.tbl"
#include "mydefs.h"
#include "MathLib.h"
#include <fenv.h>
#include <float.h>

extern double __ieee754_exp (double);

#include "eexp.tbl"

static const double
  half = 0.5,
  one = 1.0;


double
__ieee754_exp (double x_arg)
{
  double z, t;
  double retval;
  int hx, ix, k, j, m;
  int fe_val;
  union
  {
    int i_part[2];
    double x;
  } xx;
  union
  {
    int y_part[2];
    double y;
  } yy;
  xx.x = x_arg;

  ix = xx.i_part[HIGH_HALF];
  hx = ix & ~0x80000000;

  if (hx < 0x3ff0a2b2)
    {				/* |x| < 3/2 ln 2 */
      if (hx < 0x3f862e42)
	{			/* |x| < 1/64 ln 2 */
	  if (hx < 0x3ed00000)
	    {			/* |x| < 2^-18 */
	      if (hx < 0x3e300000)
		{
		  retval = one + xx.x;
		  return retval;
		}
	      retval = one + xx.x * (one + half * xx.x);
	      return retval;
	    }
	  /* Use FE_TONEAREST rounding mode for computing yy.y.
	     Avoid set/reset of rounding mode if in FE_TONEAREST mode.  */
	  fe_val = get_rounding_mode ();
	  if (fe_val == FE_TONEAREST)
	    {
	      t = xx.x * xx.x;
	      yy.y = xx.x + (t * (half + xx.x * t2)
			     + (t * t) * (t3 + xx.x * t4 + t * t5));
	      retval = one + yy.y;
	    }
	  else
	    {
	      libc_fesetround (FE_TONEAREST);
	      t = xx.x * xx.x;
	      yy.y = xx.x + (t * (half + xx.x * t2)
			     + (t * t) * (t3 + xx.x * t4 + t * t5));
	      retval = one + yy.y;
	      libc_fesetround (fe_val);
	    }
	  return retval;
	}

      /* Find the multiple of 2^-6 nearest x.  */
      k = hx >> 20;
      j = (0x00100000 | (hx & 0x000fffff)) >> (0x40c - k);
      j = (j - 1) & ~1;
      if (ix < 0)
	j += 134;
      /* Use FE_TONEAREST rounding mode for computing yy.y.
	 Avoid set/reset of rounding mode if in FE_TONEAREST mode.  */
      fe_val = get_rounding_mode ();
      if (fe_val == FE_TONEAREST)
	{
	  z = xx.x - TBL2[j];
	  t = z * z;
	  yy.y = z + (t * (half + (z * t2))
		      + (t * t) * (t3 + z * t4 + t * t5));
	  retval = TBL2[j + 1] + TBL2[j + 1] * yy.y;
	}
      else
	{
	  libc_fesetround (FE_TONEAREST);
	  z = xx.x - TBL2[j];
	  t = z * z;
	  yy.y = z + (t * (half + (z * t2))
		      + (t * t) * (t3 + z * t4 + t * t5));
	  retval = TBL2[j + 1] + TBL2[j + 1] * yy.y;
	  libc_fesetround (fe_val);
	}
      return retval;
    }

  if (hx >= 0x40862e42)
    {				/* x is large, infinite, or nan.  */
      if (hx >= 0x7ff00000)
	{
	  if (ix == 0xfff00000 && xx.i_part[LOW_HALF] == 0)
	    return zero;	/* exp(-inf) = 0.  */
	  return (xx.x * xx.x);	/* exp(nan/inf) is nan or inf.  */
	}
      if (xx.x > threshold1)
	{			/* Set overflow error condition.  */
	  retval = hhuge * hhuge;
	  return retval;
	}
      if (-xx.x > threshold2)
	{			/* Set underflow error condition.  */
	  double force_underflow = tiny * tiny;
	  math_force_eval (force_underflow);
	  retval = force_underflow;
	  return retval;
	}
    }

  /* Use FE_TONEAREST rounding mode for computing yy.y.
     Avoid set/reset of rounding mode if already in FE_TONEAREST mode.  */
  fe_val = get_rounding_mode ();
  if (fe_val == FE_TONEAREST)
    {
      t = invln2_64 * xx.x;
      if (ix < 0)
	t -= half;
      else
	t += half;
      k = (int) t;
      j = (k & 0x3f) << 1;
      m = k >> 6;
      z = (xx.x - k * ln2_64hi) - k * ln2_64lo;

      /* z is now in primary range.  */
      t = z * z;
      yy.y = z + (t * (half + z * t2) + (t * t) * (t3 + z * t4 + t * t5));
      yy.y = TBL[j] + (TBL[j + 1] + TBL[j] * yy.y);
    }
  else
    {
      libc_fesetround (FE_TONEAREST);
      t = invln2_64 * xx.x;
      if (ix < 0)
	t -= half;
      else
	t += half;
      k = (int) t;
      j = (k & 0x3f) << 1;
      m = k >> 6;
      z = (xx.x - k * ln2_64hi) - k * ln2_64lo;

      /* z is now in primary range.  */
      t = z * z;
      yy.y = z + (t * (half + z * t2) + (t * t) * (t3 + z * t4 + t * t5));
      yy.y = TBL[j] + (TBL[j + 1] + TBL[j] * yy.y);
      libc_fesetround (fe_val);
    }

  if (m < -1021)
    {
      yy.y_part[HIGH_HALF] += (m + 54) << 20;
      retval = twom54 * yy.y;
      if (retval < DBL_MIN)
	{
	  double force_underflow = tiny * tiny;
	  math_force_eval (force_underflow);
	}
      return retval;
    }
  yy.y_part[HIGH_HALF] += m << 20;
  return yy.y;
}
#ifndef __ieee754_exp
strong_alias (__ieee754_exp, __exp_finite)
#endif

#ifndef SECTION
# define SECTION
#endif

/* Compute e^(x+xx).  The routine also receives bound of error of previous
   calculation.  If after computing exp the error exceeds the allowed bounds,
   the routine returns a non-positive number.  Otherwise it returns the
   computed result, which is always positive.  */
double
SECTION
__exp1 (double x, double xx, double error)
{
  double bexp, t, eps, del, base, y, al, bet, res, rem, cor;
  mynumber junk1, junk2, binexp = {{0, 0}};
  int4 i, j, m, n, ex;

  junk1.x = x;
  m = junk1.i[HIGH_HALF];
  n = m & hugeint;		/* no sign */

  if (n > smallint && n < bigint)
    {
      y = x * log2e.x + three51.x;
      bexp = y - three51.x;	/*  multiply the result by 2**bexp        */

      junk1.x = y;

      eps = bexp * ln_two2.x;	/* x = bexp*ln(2) + t - eps               */
      t = x - bexp * ln_two1.x;

      y = t + three33.x;
      base = y - three33.x;	/* t rounded to a multiple of 2**-18      */
      junk2.x = y;
      del = (t - base) + (xx - eps);	/*  x = bexp*ln(2) + base + del      */
      eps = del + del * del * (p3.x * del + p2.x);

      binexp.i[HIGH_HALF] = (junk1.i[LOW_HALF] + 1023) << 20;

      i = ((junk2.i[LOW_HALF] >> 8) & 0xfffffffe) + 356;
      j = (junk2.i[LOW_HALF] & 511) << 1;

      al = coar.x[i] * fine.x[j];
      bet = ((coar.x[i] * fine.x[j + 1] + coar.x[i + 1] * fine.x[j])
	     + coar.x[i + 1] * fine.x[j + 1]);

      rem = (bet + bet * eps) + al * eps;
      res = al + rem;
      cor = (al - res) + rem;
      if (res == (res + cor * (1.0 + error + err_1)))
	return res * binexp.x;
      else
	return -10.0;
    }

  if (n <= smallint)
    return 1.0;			/*  if x->0 e^x=1 */

  if (n >= badint)
    {
      if (n > infint)
	return (zero / zero);	/* x is NaN,  return invalid */
      if (n < infint)
	return ((x > 0) ? (hhuge * hhuge) : (tiny * tiny));
      /* x is finite,  cause either overflow or underflow  */
      if (junk1.i[LOW_HALF] != 0)
	return (zero / zero);	/*  x is NaN  */
      return ((x > 0) ? inf.x : zero);	/* |x| = inf;  return either inf or 0 */
    }

  y = x * log2e.x + three51.x;
  bexp = y - three51.x;
  junk1.x = y;
  eps = bexp * ln_two2.x;
  t = x - bexp * ln_two1.x;
  y = t + three33.x;
  base = y - three33.x;
  junk2.x = y;
  del = (t - base) + (xx - eps);
  eps = del + del * del * (p3.x * del + p2.x);
  i = ((junk2.i[LOW_HALF] >> 8) & 0xfffffffe) + 356;
  j = (junk2.i[LOW_HALF] & 511) << 1;
  al = coar.x[i] * fine.x[j];
  bet = ((coar.x[i] * fine.x[j + 1] + coar.x[i + 1] * fine.x[j])
	 + coar.x[i + 1] * fine.x[j + 1]);
  rem = (bet + bet * eps) + al * eps;
  res = al + rem;
  cor = (al - res) + rem;
  if (m >> 31)
    {
      ex = junk1.i[LOW_HALF];
      if (res < 1.0)
	{
	  res += res;
	  cor += cor;
	  ex -= 1;
	}
      if (ex >= -1022)
	{
	  binexp.i[HIGH_HALF] = (1023 + ex) << 20;
	  if (res == (res + cor * (1.0 + error + err_1)))
	    return res * binexp.x;
	  else
	    return -10.0;
	}
      ex = -(1022 + ex);
      binexp.i[HIGH_HALF] = (1023 - ex) << 20;
      res *= binexp.x;
      cor *= binexp.x;
      eps = 1.00000000001 + (error + err_1) * binexp.x;
      t = 1.0 + res;
      y = ((1.0 - t) + res) + cor;
      res = t + y;
      cor = (t - res) + y;
      if (res == (res + eps * cor))
	{
	  binexp.i[HIGH_HALF] = 0x00100000;
	  return (res - 1.0) * binexp.x;
	}
      else
	return -10.0;
    }
  else
    {
      binexp.i[HIGH_HALF] = (junk1.i[LOW_HALF] + 767) << 20;
      if (res == (res + cor * (1.0 + error + err_1)))
	return res * binexp.x * t256.x;
      else
	return -10.0;
    }
}
