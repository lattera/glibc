/*
 * IBM Accurate Mathematical Library
 * written by International Business Machines Corp.
 * Copyright (C) 2001-2015 Free Software Foundation, Inc.
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
 * You should have received a copy of the GNU  Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
/****************************************************************************/
/*                                                                          */
/* MODULE_NAME:usncs.c                                                      */
/*                                                                          */
/* FUNCTIONS: usin                                                          */
/*            ucos                                                          */
/*            slow                                                          */
/*            slow1                                                         */
/*            slow2                                                         */
/*            sloww                                                         */
/*            sloww1                                                        */
/*            sloww2                                                        */
/*            bsloww                                                        */
/*            bsloww1                                                       */
/*            bsloww2                                                       */
/*            cslow2                                                        */
/*            csloww                                                        */
/*            csloww1                                                       */
/*            csloww2                                                       */
/* FILES NEEDED: dla.h endian.h mpa.h mydefs.h  usncs.h                     */
/*               branred.c sincos32.c dosincos.c mpa.c                      */
/*               sincos.tbl                                                 */
/*                                                                          */
/* An ultimate sin and  routine. Given an IEEE double machine number x       */
/* it computes the correctly rounded (to nearest) value of sin(x) or cos(x) */
/* Assumption: Machine arithmetic operations are performed in               */
/* round to nearest mode of IEEE 754 standard.                              */
/*                                                                          */
/****************************************************************************/


#include <errno.h>
#include "endian.h"
#include "mydefs.h"
#include "usncs.h"
#include "MathLib.h"
#include <math_private.h>
#include <fenv.h>

/* Helper macros to compute sin of the input values.  */
#define POLYNOMIAL2(xx) ((((s5 * (xx) + s4) * (xx) + s3) * (xx) + s2) * (xx))

#define POLYNOMIAL(xx) (POLYNOMIAL2 (xx) + s1)

/* The computed polynomial is a variation of the Taylor series expansion for
   sin(a):

   a - a^3/3! + a^5/5! - a^7/7! + a^9/9! + (1 - a^2) * da / 2

   The constants s1, s2, s3, etc. are pre-computed values of 1/3!, 1/5! and so
   on.  The result is returned to LHS and correction in COR.  */
#define TAYLOR_SIN(xx, a, da, cor) \
({									      \
  double t = ((POLYNOMIAL (xx)  * (a) - 0.5 * (da))  * (xx) + (da));	      \
  double res = (a) + t;							      \
  (cor) = ((a) - res) + t;						      \
  res;									      \
})

/* This is again a variation of the Taylor series expansion with the term
   x^3/3! expanded into the following for better accuracy:

   bb * x ^ 3 + 3 * aa * x * x1 * x2 + aa * x1 ^ 3 + aa * x2 ^ 3

   The correction term is dx and bb + aa = -1/3!
   */
#define TAYLOR_SLOW(x0, dx, cor) \
({									      \
  static const double th2_36 = 206158430208.0;	/*    1.5*2**37   */	      \
  double xx = (x0) * (x0);						      \
  double x1 = ((x0) + th2_36) - th2_36;					      \
  double y = aa * x1 * x1 * x1;						      \
  double r = (x0) + y;							      \
  double x2 = ((x0) - x1) + (dx);					      \
  double t = (((POLYNOMIAL2 (xx) + bb) * xx + 3.0 * aa * x1 * x2)	      \
	      * (x0)  + aa * x2 * x2 * x2 + (dx));			      \
  t = (((x0) - r) + y) + t;						      \
  double res = r + t;							      \
  (cor) = (r - res) + t;						      \
  res;									      \
})

#define SINCOS_TABLE_LOOKUP(u, sn, ssn, cs, ccs) \
({									      \
  int4 k = u.i[LOW_HALF] << 2;						      \
  sn = __sincostab.x[k];						      \
  ssn = __sincostab.x[k + 1];						      \
  cs = __sincostab.x[k + 2];						      \
  ccs = __sincostab.x[k + 3];						      \
})

#ifndef SECTION
# define SECTION
#endif

extern const union
{
  int4 i[880];
  double x[440];
} __sincostab attribute_hidden;

static const double
  sn3 = -1.66666666666664880952546298448555E-01,
  sn5 = 8.33333214285722277379541354343671E-03,
  cs2 = 4.99999999999999999999950396842453E-01,
  cs4 = -4.16666666666664434524222570944589E-02,
  cs6 = 1.38888874007937613028114285595617E-03;

static const double t22 = 0x1.8p22;

void __dubsin (double x, double dx, double w[]);
void __docos (double x, double dx, double w[]);
double __mpsin (double x, double dx, bool reduce_range);
double __mpcos (double x, double dx, bool reduce_range);
static double slow (double x);
static double slow1 (double x);
static double slow2 (double x);
static double sloww (double x, double dx, double orig);
static double sloww1 (double x, double dx, double orig, int m);
static double sloww2 (double x, double dx, double orig, int n);
static double bsloww (double x, double dx, double orig, int n);
static double bsloww1 (double x, double dx, double orig, int n);
static double bsloww2 (double x, double dx, double orig, int n);
int __branred (double x, double *a, double *aa);
static double cslow2 (double x);
static double csloww (double x, double dx, double orig);
static double csloww1 (double x, double dx, double orig, int m);
static double csloww2 (double x, double dx, double orig, int n);

/* Given a number partitioned into U and X such that U is an index into the
   sin/cos table, this macro computes the cosine of the number by combining
   the sin and cos of X (as computed by a variation of the Taylor series) with
   the values looked up from the sin/cos table to get the result in RES and a
   correction value in COR.  */
static double
do_cos (mynumber u, double x, double *corp)
{
  double xx, s, sn, ssn, c, cs, ccs, res, cor;
  xx = x * x;
  s = x + x * xx * (sn3 + xx * sn5);
  c = xx * (cs2 + xx * (cs4 + xx * cs6));
  SINCOS_TABLE_LOOKUP (u, sn, ssn, cs, ccs);
  cor = (ccs - s * ssn - cs * c) - sn * s;
  res = cs + cor;
  cor = (cs - res) + cor;
  *corp = cor;
  return res;
}

/* A more precise variant of DO_COS where the number is partitioned into U, X
   and DX.  EPS is the adjustment to the correction COR.  */
static double
do_cos_slow (mynumber u, double x, double dx, double eps, double *corp)
{
  double xx, y, x1, x2, e1, e2, res, cor;
  double s, sn, ssn, c, cs, ccs;
  xx = x * x;
  s = x * xx * (sn3 + xx * sn5);
  c = x * dx + xx * (cs2 + xx * (cs4 + xx * cs6));
  SINCOS_TABLE_LOOKUP (u, sn, ssn, cs, ccs);
  x1 = (x + t22) - t22;
  x2 = (x - x1) + dx;
  e1 = (sn + t22) - t22;
  e2 = (sn - e1) + ssn;
  cor = (ccs - cs * c - e1 * x2 - e2 * x) - sn * s;
  y = cs - e1 * x1;
  cor = cor + ((cs - y) - e1 * x1);
  res = y + cor;
  cor = (y - res) + cor;
  if (cor > 0)
    cor = 1.0005 * cor + eps;
  else
    cor = 1.0005 * cor - eps;
  *corp = cor;
  return res;
}

/* Given a number partitioned into U and X and DX such that U is an index into
   the sin/cos table, this macro computes the sine of the number by combining
   the sin and cos of X (as computed by a variation of the Taylor series) with
   the values looked up from the sin/cos table to get the result in RES and a
   correction value in COR.  */
static double
do_sin (mynumber u, double x, double dx, double *corp)
{
  double xx, s, sn, ssn, c, cs, ccs, cor, res;
  xx = x * x;
  s = x + (dx + x * xx * (sn3 + xx * sn5));
  c = x * dx + xx * (cs2 + xx * (cs4 + xx * cs6));
  SINCOS_TABLE_LOOKUP (u, sn, ssn, cs, ccs);
  cor = (ssn + s * ccs - sn * c) + cs * s;
  res = sn + cor;
  cor = (sn - res) + cor;
  *corp = cor;
  return res;
}

/* A more precise variant of res = do_sin where the number is partitioned into U, X
   and DX.  EPS is the adjustment to the correction COR.  */
static double
do_sin_slow (mynumber u, double x, double dx, double eps, double *corp)
{
  double xx, y, x1, x2, c1, c2, res, cor;
  double s, sn, ssn, c, cs, ccs;
  xx = x * x;
  s = x * xx * (sn3 + xx * sn5);
  c = xx * (cs2 + xx * (cs4 + xx * cs6));
  SINCOS_TABLE_LOOKUP (u, sn, ssn, cs, ccs);
  x1 = (x + t22) - t22;
  x2 = (x - x1) + dx;
  c1 = (cs + t22) - t22;
  c2 = (cs - c1) + ccs;
  cor = (ssn + s * ccs + cs * s + c2 * x + c1 * x2 - sn * x * dx) - sn * c;
  y = sn + c1 * x1;
  cor = cor + ((sn - y) + c1 * x1);
  res = y + cor;
  cor = (y - res) + cor;
  if (cor > 0)
    cor = 1.0005 * cor + eps;
  else
    cor = 1.0005 * cor - eps;
  *corp = cor;
  return res;
}

/* Reduce range of X and compute sin of a + da.  K is the amount by which to
   rotate the quadrants.  This allows us to use the same routine to compute cos
   by simply rotating the quadrants by 1.  */
static inline double
__always_inline
reduce_and_compute (double x, unsigned int k)
{
  double retval = 0, a, da;
  unsigned int n = __branred (x, &a, &da);
  k = (n + k) % 4;
  switch (k)
    {
      case 0:
	if (a * a < 0.01588)
	  retval = bsloww (a, da, x, n);
	else
	  retval = bsloww1 (a, da, x, n);
	break;
      case 2:
	if (a * a < 0.01588)
	  retval = bsloww (-a, -da, x, n);
	else
	  retval = bsloww1 (-a, -da, x, n);
	break;

      case 1:
      case 3:
	retval = bsloww2 (a, da, x, n);
	break;
    }
  return retval;
}

/*******************************************************************/
/* An ultimate sin routine. Given an IEEE double machine number x   */
/* it computes the correctly rounded (to nearest) value of sin(x)  */
/*******************************************************************/
double
SECTION
__sin (double x)
{
  double xx, res, t, cor, y, s, c, sn, ssn, cs, ccs, xn, a, da, db, eps, xn1,
    xn2;
  mynumber u, v;
  int4 k, m, n;
  double retval = 0;

  SET_RESTORE_ROUND_53BIT (FE_TONEAREST);

  u.x = x;
  m = u.i[HIGH_HALF];
  k = 0x7fffffff & m;		/* no sign           */
  if (k < 0x3e500000)		/* if x->0 =>sin(x)=x */
    retval = x;
 /*---------------------------- 2^-26 < |x|< 0.25 ----------------------*/
  else if (k < 0x3fd00000)
    {
      xx = x * x;
      /* Taylor series.  */
      t = POLYNOMIAL (xx) * (xx * x);
      res = x + t;
      cor = (x - res) + t;
      retval = (res == res + 1.07 * cor) ? res : slow (x);
    }				/*  else  if (k < 0x3fd00000)    */
/*---------------------------- 0.25<|x|< 0.855469---------------------- */
  else if (k < 0x3feb6000)
    {
      u.x = (m > 0) ? big + x : big - x;
      y = (m > 0) ? x - (u.x - big) : x + (u.x - big);
      xx = y * y;
      s = y + y * xx * (sn3 + xx * sn5);
      c = xx * (cs2 + xx * (cs4 + xx * cs6));
      SINCOS_TABLE_LOOKUP (u, sn, ssn, cs, ccs);
      if (m <= 0)
        {
          sn = -sn;
	  ssn = -ssn;
	}
      cor = (ssn + s * ccs - sn * c) + cs * s;
      res = sn + cor;
      cor = (sn - res) + cor;
      retval = (res == res + 1.096 * cor) ? res : slow1 (x);
    }				/*   else  if (k < 0x3feb6000)    */

/*----------------------- 0.855469  <|x|<2.426265  ----------------------*/
  else if (k < 0x400368fd)
    {

      y = (m > 0) ? hp0 - x : hp0 + x;
      if (y >= 0)
	{
	  u.x = big + y;
	  y = (y - (u.x - big)) + hp1;
	}
      else
	{
	  u.x = big - y;
	  y = (-hp1) - (y + (u.x - big));
	}
      res = do_cos (u, y, &cor);
      retval = (res == res + 1.020 * cor) ? ((m > 0) ? res : -res) : slow2 (x);
    }				/*   else  if (k < 0x400368fd)    */

/*-------------------------- 2.426265<|x|< 105414350 ----------------------*/
  else if (k < 0x419921FB)
    {
      t = (x * hpinv + toint);
      xn = t - toint;
      v.x = t;
      y = (x - xn * mp1) - xn * mp2;
      n = v.i[LOW_HALF] & 3;
      da = xn * mp3;
      a = y - da;
      da = (y - a) - da;
      eps = ABS (x) * 1.2e-30;

      switch (n)
	{			/* quarter of unit circle */
	case 0:
	case 2:
	  xx = a * a;
	  if (n)
	    {
	      a = -a;
	      da = -da;
	    }
	  if (xx < 0.01588)
	    {
	      /* Taylor series.  */
	      res = TAYLOR_SIN (xx, a, da, cor);
	      cor = (cor > 0) ? 1.02 * cor + eps : 1.02 * cor - eps;
	      retval = (res == res + cor) ? res : sloww (a, da, x);
	    }
	  else
	    {
	      if (a > 0)
		m = 1;
	      else
		{
		  m = 0;
		  a = -a;
		  da = -da;
		}
	      u.x = big + a;
	      y = a - (u.x - big);
	      res = do_sin (u, y, da, &cor);
	      cor = (cor > 0) ? 1.035 * cor + eps : 1.035 * cor - eps;
	      retval = ((res == res + cor) ? ((m) ? res : -res)
			: sloww1 (a, da, x, m));
	    }
	  break;

	case 1:
	case 3:
	  if (a < 0)
	    {
	      a = -a;
	      da = -da;
	    }
	  u.x = big + a;
	  y = a - (u.x - big) + da;
	  res = do_cos (u, y, &cor);
	  cor = (cor > 0) ? 1.025 * cor + eps : 1.025 * cor - eps;
	  retval = ((res == res + cor) ? ((n & 2) ? -res : res)
		    : sloww2 (a, da, x, n));
	  break;
	}
    }				/*   else  if (k <  0x419921FB )    */

/*---------------------105414350 <|x|< 281474976710656 --------------------*/
  else if (k < 0x42F00000)
    {
      t = (x * hpinv + toint);
      xn = t - toint;
      v.x = t;
      xn1 = (xn + 8.0e22) - 8.0e22;
      xn2 = xn - xn1;
      y = ((((x - xn1 * mp1) - xn1 * mp2) - xn2 * mp1) - xn2 * mp2);
      n = v.i[LOW_HALF] & 3;
      da = xn1 * pp3;
      t = y - da;
      da = (y - t) - da;
      da = (da - xn2 * pp3) - xn * pp4;
      a = t + da;
      da = (t - a) + da;
      eps = 1.0e-24;

      switch (n)
	{
	case 0:
	case 2:
	  xx = a * a;
	  if (n)
	    {
	      a = -a;
	      da = -da;
	    }
	  if (xx < 0.01588)
	    {
	      /* Taylor series.  */
	      res = TAYLOR_SIN (xx, a, da, cor);
	      cor = (cor > 0) ? 1.02 * cor + eps : 1.02 * cor - eps;
	      retval = (res == res + cor) ? res : bsloww (a, da, x, n);
	    }
	  else
	    {
	      double t;
	      if (a > 0)
		{
		  m = 1;
		  t = a;
		  db = da;
		}
	      else
		{
		  m = 0;
		  t = -a;
		  db = -da;
		}
	      u.x = big + t;
	      y = t - (u.x - big);
	      res = do_sin (u, y, db, &cor);
	      cor = (cor > 0) ? 1.035 * cor + eps : 1.035 * cor - eps;
	      retval = ((res == res + cor) ? ((m) ? res : -res)
			: bsloww1 (a, da, x, n));
	    }
	  break;

	case 1:
	case 3:
	  if (a < 0)
	    {
	      a = -a;
	      da = -da;
	    }
	  u.x = big + a;
	  y = a - (u.x - big) + da;
	  res = do_cos (u, y, &cor);
	  cor = (cor > 0) ? 1.025 * cor + eps : 1.025 * cor - eps;
	  retval = ((res == res + cor) ? ((n & 2) ? -res : res)
		    : bsloww2 (a, da, x, n));
	  break;
	}
    }				/*   else  if (k <  0x42F00000 )   */

/* -----------------281474976710656 <|x| <2^1024----------------------------*/
  else if (k < 0x7ff00000)
    retval = reduce_and_compute (x, 0);

/*--------------------- |x| > 2^1024 ----------------------------------*/
  else
    {
      if (k == 0x7ff00000 && u.i[LOW_HALF] == 0)
	__set_errno (EDOM);
      retval = x / x;
    }

  return retval;
}


/*******************************************************************/
/* An ultimate cos routine. Given an IEEE double machine number x   */
/* it computes the correctly rounded (to nearest) value of cos(x)  */
/*******************************************************************/

double
SECTION
__cos (double x)
{
  double y, xx, res, t, cor, xn, a, da, db, eps, xn1,
    xn2;
  mynumber u, v;
  int4 k, m, n;

  double retval = 0;

  SET_RESTORE_ROUND_53BIT (FE_TONEAREST);

  u.x = x;
  m = u.i[HIGH_HALF];
  k = 0x7fffffff & m;

  /* |x|<2^-27 => cos(x)=1 */
  if (k < 0x3e400000)
    retval = 1.0;

  else if (k < 0x3feb6000)
    {				/* 2^-27 < |x| < 0.855469 */
      y = ABS (x);
      u.x = big + y;
      y = y - (u.x - big);
      res = do_cos (u, y, &cor);
      retval = (res == res + 1.020 * cor) ? res : cslow2 (x);
    }				/*   else  if (k < 0x3feb6000)    */

  else if (k < 0x400368fd)
    { /* 0.855469  <|x|<2.426265  */ ;
      y = hp0 - ABS (x);
      a = y + hp1;
      da = (y - a) + hp1;
      xx = a * a;
      if (xx < 0.01588)
	{
	  res = TAYLOR_SIN (xx, a, da, cor);
	  cor = (cor > 0) ? 1.02 * cor + 1.0e-31 : 1.02 * cor - 1.0e-31;
	  retval = (res == res + cor) ? res : csloww (a, da, x);
	}
      else
	{
	  if (a > 0)
	    {
	      m = 1;
	    }
	  else
	    {
	      m = 0;
	      a = -a;
	      da = -da;
	    }
	  u.x = big + a;
	  y = a - (u.x - big);
	  res = do_sin (u, y, da, &cor);
	  cor = (cor > 0) ? 1.035 * cor + 1.0e-31 : 1.035 * cor - 1.0e-31;
	  retval = ((res == res + cor) ? ((m) ? res : -res)
		    : csloww1 (a, da, x, m));
	}

    }				/*   else  if (k < 0x400368fd)    */


  else if (k < 0x419921FB)
    {				/* 2.426265<|x|< 105414350 */
      t = (x * hpinv + toint);
      xn = t - toint;
      v.x = t;
      y = (x - xn * mp1) - xn * mp2;
      n = v.i[LOW_HALF] & 3;
      da = xn * mp3;
      a = y - da;
      da = (y - a) - da;
      eps = ABS (x) * 1.2e-30;

      switch (n)
	{
	case 1:
	case 3:
	  xx = a * a;
	  if (n == 1)
	    {
	      a = -a;
	      da = -da;
	    }
	  if (xx < 0.01588)
	    {
	      res = TAYLOR_SIN (xx, a, da, cor);
	      cor = (cor > 0) ? 1.02 * cor + eps : 1.02 * cor - eps;
	      retval = (res == res + cor) ? res : csloww (a, da, x);
	    }
	  else
	    {
	      if (a > 0)
		{
		  m = 1;
		}
	      else
		{
		  m = 0;
		  a = -a;
		  da = -da;
		}
	      u.x = big + a;
	      y = a - (u.x - big);
	      res = do_sin (u, y, da, &cor);
	      cor = (cor > 0) ? 1.035 * cor + eps : 1.035 * cor - eps;
	      retval = ((res == res + cor) ? ((m) ? res : -res)
			: csloww1 (a, da, x, m));
	    }
	  break;

	case 0:
	case 2:
	  if (a < 0)
	    {
	      a = -a;
	      da = -da;
	    }
	  u.x = big + a;
	  y = a - (u.x - big) + da;
	  res = do_cos (u, y, &cor);
	  cor = (cor > 0) ? 1.025 * cor + eps : 1.025 * cor - eps;
	  retval = ((res == res + cor) ? ((n) ? -res : res)
		    : csloww2 (a, da, x, n));
	  break;
	}
    }				/*   else  if (k <  0x419921FB )    */

  else if (k < 0x42F00000)
    {
      t = (x * hpinv + toint);
      xn = t - toint;
      v.x = t;
      xn1 = (xn + 8.0e22) - 8.0e22;
      xn2 = xn - xn1;
      y = ((((x - xn1 * mp1) - xn1 * mp2) - xn2 * mp1) - xn2 * mp2);
      n = v.i[LOW_HALF] & 3;
      da = xn1 * pp3;
      t = y - da;
      da = (y - t) - da;
      da = (da - xn2 * pp3) - xn * pp4;
      a = t + da;
      da = (t - a) + da;
      eps = 1.0e-24;

      switch (n)
	{
	case 1:
	case 3:
	  xx = a * a;
	  if (n == 1)
	    {
	      a = -a;
	      da = -da;
	    }
	  if (xx < 0.01588)
	    {
	      res = TAYLOR_SIN (xx, a, da, cor);
	      cor = (cor > 0) ? 1.02 * cor + eps : 1.02 * cor - eps;
	      retval = (res == res + cor) ? res : bsloww (a, da, x, n);
	    }
	  else
	    {
	      double t;
	      if (a > 0)
		{
		  m = 1;
		  t = a;
		  db = da;
		}
	      else
		{
		  m = 0;
		  t = -a;
		  db = -da;
		}
	      u.x = big + t;
	      y = t - (u.x - big);
	      res = do_sin (u, y, db, &cor);
	      cor = (cor > 0) ? 1.035 * cor + eps : 1.035 * cor - eps;
	      retval = ((res == res + cor) ? ((m) ? res : -res)
			: bsloww1 (a, da, x, n));
	    }
	  break;

	case 0:
	case 2:
	  if (a < 0)
	    {
	      a = -a;
	      da = -da;
	    }
	  u.x = big + a;
	  y = a - (u.x - big) + da;
	  res = do_cos (u, y, &cor);
	  cor = (cor > 0) ? 1.025 * cor + eps : 1.025 * cor - eps;
	  retval = ((res == res + cor) ? ((n) ? -res : res)
		    : bsloww2 (a, da, x, n));
	  break;
	}
    }				/*   else  if (k <  0x42F00000 )    */

  /* 281474976710656 <|x| <2^1024 */
  else if (k < 0x7ff00000)
    retval = reduce_and_compute (x, 1);

  else
    {
      if (k == 0x7ff00000 && u.i[LOW_HALF] == 0)
	__set_errno (EDOM);
      retval = x / x;		/* |x| > 2^1024 */
    }

  return retval;
}

/************************************************************************/
/*  Routine compute sin(x) for  2^-26 < |x|< 0.25 by  Taylor with more   */
/* precision  and if still doesn't accurate enough by mpsin   or dubsin */
/************************************************************************/

static double
SECTION
slow (double x)
{
  double res, cor, w[2];
  res = TAYLOR_SLOW (x, 0, cor);
  if (res == res + 1.0007 * cor)
    return res;
  else
    {
      __dubsin (ABS (x), 0, w);
      if (w[0] == w[0] + 1.000000001 * w[1])
	return (x > 0) ? w[0] : -w[0];
      else
	return (x > 0) ? __mpsin (x, 0, false) : -__mpsin (-x, 0, false);
    }
}

/*******************************************************************************/
/* Routine compute sin(x) for 0.25<|x|< 0.855469 by __sincostab.tbl and Taylor */
/* and if result still doesn't accurate enough by mpsin   or dubsin            */
/*******************************************************************************/

static double
SECTION
slow1 (double x)
{
  mynumber u;
  double w[2], y, cor, res;
  y = ABS (x);
  u.x = big + y;
  y = y - (u.x - big);
  res = do_sin_slow (u, y, 0, 0, &cor);
  if (res == res + cor)
    return (x > 0) ? res : -res;
  else
    {
      __dubsin (ABS (x), 0, w);
      if (w[0] == w[0] + 1.000000005 * w[1])
	return (x > 0) ? w[0] : -w[0];
      else
	return (x > 0) ? __mpsin (x, 0, false) : -__mpsin (-x, 0, false);
    }
}

/**************************************************************************/
/*  Routine compute sin(x) for   0.855469  <|x|<2.426265  by  __sincostab.tbl  */
/* and if result still doesn't accurate enough by mpsin   or dubsin       */
/**************************************************************************/
static double
SECTION
slow2 (double x)
{
  mynumber u;
  double w[2], y, y1, y2, cor, res, del;

  y = ABS (x);
  y = hp0 - y;
  if (y >= 0)
    {
      u.x = big + y;
      y = y - (u.x - big);
      del = hp1;
    }
  else
    {
      u.x = big - y;
      y = -(y + (u.x - big));
      del = -hp1;
    }
  res = do_cos_slow (u, y, del, 0, &cor);
  if (res == res + cor)
    return (x > 0) ? res : -res;
  else
    {
      y = ABS (x) - hp0;
      y1 = y - hp1;
      y2 = (y - y1) - hp1;
      __docos (y1, y2, w);
      if (w[0] == w[0] + 1.000000005 * w[1])
	return (x > 0) ? w[0] : -w[0];
      else
	return (x > 0) ? __mpsin (x, 0, false) : -__mpsin (-x, 0, false);
    }
}

/***************************************************************************/
/*  Routine compute sin(x+dx) (Double-Length number) where x is small enough*/
/* to use Taylor series around zero and   (x+dx)                            */
/* in first or third quarter of unit circle.Routine receive also            */
/* (right argument) the  original   value of x for computing error of      */
/* result.And if result not accurate enough routine calls mpsin1 or dubsin */
/***************************************************************************/

static double
SECTION
sloww (double x, double dx, double orig)
{
  double y, t, res, cor, w[2], a, da, xn;
  mynumber v;
  int4 n;
  res = TAYLOR_SLOW (x, dx, cor);
  if (cor > 0)
    cor = 1.0005 * cor + ABS (orig) * 3.1e-30;
  else
    cor = 1.0005 * cor - ABS (orig) * 3.1e-30;

  if (res == res + cor)
    return res;
  else
    {
      (x > 0) ? __dubsin (x, dx, w) : __dubsin (-x, -dx, w);
      if (w[1] > 0)
	cor = 1.000000001 * w[1] + ABS (orig) * 1.1e-30;
      else
	cor = 1.000000001 * w[1] - ABS (orig) * 1.1e-30;

      if (w[0] == w[0] + cor)
	return (x > 0) ? w[0] : -w[0];
      else
	{
	  t = (orig * hpinv + toint);
	  xn = t - toint;
	  v.x = t;
	  y = (orig - xn * mp1) - xn * mp2;
	  n = v.i[LOW_HALF] & 3;
	  da = xn * pp3;
	  t = y - da;
	  da = (y - t) - da;
	  y = xn * pp4;
	  a = t - y;
	  da = ((t - a) - y) + da;
	  if (n & 2)
	    {
	      a = -a;
	      da = -da;
	    }
	  (a > 0) ? __dubsin (a, da, w) : __dubsin (-a, -da, w);
	  if (w[1] > 0)
	    cor = 1.000000001 * w[1] + ABS (orig) * 1.1e-40;
	  else
	    cor = 1.000000001 * w[1] - ABS (orig) * 1.1e-40;

	  if (w[0] == w[0] + cor)
	    return (a > 0) ? w[0] : -w[0];
	  else
	    return __mpsin (orig, 0, true);
	}
    }
}

/***************************************************************************/
/*  Routine compute sin(x+dx)   (Double-Length number) where x in first or  */
/*  third quarter of unit circle.Routine receive also (right argument) the  */
/*  original   value of x for computing error of result.And if result not  */
/* accurate enough routine calls  mpsin1   or dubsin                       */
/***************************************************************************/

static double
SECTION
sloww1 (double x, double dx, double orig, int m)
{
  mynumber u;
  double w[2], y, cor, res;

  u.x = big + x;
  y = x - (u.x - big);
  res = do_sin_slow (u, y, dx, 3.1e-30 * ABS (orig), &cor);

  if (res == res + cor)
    return (m > 0) ? res : -res;
  else
    {
      __dubsin (x, dx, w);

      if (w[1] > 0)
	cor = 1.000000005 * w[1] + 1.1e-30 * ABS (orig);
      else
	cor = 1.000000005 * w[1] - 1.1e-30 * ABS (orig);

      if (w[0] == w[0] + cor)
	return (m > 0) ? w[0] : -w[0];
      else
	return __mpsin (orig, 0, true);
    }
}

/***************************************************************************/
/*  Routine compute sin(x+dx)   (Double-Length number) where x in second or */
/*  fourth quarter of unit circle.Routine receive also  the  original value */
/* and quarter(n= 1or 3)of x for computing error of result.And if result not*/
/* accurate enough routine calls  mpsin1   or dubsin                       */
/***************************************************************************/

static double
SECTION
sloww2 (double x, double dx, double orig, int n)
{
  mynumber u;
  double w[2], y, cor, res;

  u.x = big + x;
  y = x - (u.x - big);
  res = do_cos_slow (u, y, dx, 3.1e-30 * ABS (orig), &cor);

  if (res == res + cor)
    return (n & 2) ? -res : res;
  else
    {
      __docos (x, dx, w);

      if (w[1] > 0)
	cor = 1.000000005 * w[1] + 1.1e-30 * ABS (orig);
      else
	cor = 1.000000005 * w[1] - 1.1e-30 * ABS (orig);

      if (w[0] == w[0] + cor)
	return (n & 2) ? -w[0] : w[0];
      else
	return __mpsin (orig, 0, true);
    }
}

/***************************************************************************/
/*  Routine compute sin(x+dx) or cos(x+dx) (Double-Length number) where x   */
/* is small enough to use Taylor series around zero and   (x+dx)            */
/* in first or third quarter of unit circle.Routine receive also            */
/* (right argument) the  original   value of x for computing error of      */
/* result.And if result not accurate enough routine calls other routines    */
/***************************************************************************/

static double
SECTION
bsloww (double x, double dx, double orig, int n)
{
  double res, cor, w[2];

  res = TAYLOR_SLOW (x, dx, cor);
  cor = (cor > 0) ? 1.0005 * cor + 1.1e-24 : 1.0005 * cor - 1.1e-24;
  if (res == res + cor)
    return res;
  else
    {
      (x > 0) ? __dubsin (x, dx, w) : __dubsin (-x, -dx, w);
      if (w[1] > 0)
	cor = 1.000000001 * w[1] + 1.1e-24;
      else
	cor = 1.000000001 * w[1] - 1.1e-24;
      if (w[0] == w[0] + cor)
	return (x > 0) ? w[0] : -w[0];
      else
	return (n & 1) ? __mpcos (orig, 0, true) : __mpsin (orig, 0, true);
    }
}

/***************************************************************************/
/*  Routine compute sin(x+dx)  or cos(x+dx) (Double-Length number) where x  */
/* in first or third quarter of unit circle.Routine receive also            */
/* (right argument) the original  value of x for computing error of result.*/
/* And if result not  accurate enough routine calls  other routines         */
/***************************************************************************/

static double
SECTION
bsloww1 (double x, double dx, double orig, int n)
{
  mynumber u;
  double w[2], y, cor, res;

  y = ABS (x);
  u.x = big + y;
  y = y - (u.x - big);
  dx = (x > 0) ? dx : -dx;
  res = do_sin_slow (u, y, dx, 1.1e-24, &cor);
  if (res == res + cor)
    return (x > 0) ? res : -res;
  else
    {
      __dubsin (ABS (x), dx, w);

      if (w[1] > 0)
	cor = 1.000000005 * w[1] + 1.1e-24;
      else
	cor = 1.000000005 * w[1] - 1.1e-24;

      if (w[0] == w[0] + cor)
	return (x > 0) ? w[0] : -w[0];
      else
	return (n & 1) ? __mpcos (orig, 0, true) : __mpsin (orig, 0, true);
    }
}

/***************************************************************************/
/*  Routine compute sin(x+dx)  or cos(x+dx) (Double-Length number) where x  */
/* in second or fourth quarter of unit circle.Routine receive also  the     */
/* original value and quarter(n= 1or 3)of x for computing error of result.  */
/* And if result not accurate enough routine calls  other routines          */
/***************************************************************************/

static double
SECTION
bsloww2 (double x, double dx, double orig, int n)
{
  mynumber u;
  double w[2], y, cor, res;

  y = ABS (x);
  u.x = big + y;
  y = y - (u.x - big);
  dx = (x > 0) ? dx : -dx;
  res = do_cos_slow (u, y, dx, 1.1e-24, &cor);
  if (res == res + cor)
    return (n & 2) ? -res : res;
  else
    {
      __docos (ABS (x), dx, w);

      if (w[1] > 0)
	cor = 1.000000005 * w[1] + 1.1e-24;
      else
	cor = 1.000000005 * w[1] - 1.1e-24;

      if (w[0] == w[0] + cor)
	return (n & 2) ? -w[0] : w[0];
      else
	return (n & 1) ? __mpsin (orig, 0, true) : __mpcos (orig, 0, true);
    }
}

/************************************************************************/
/*  Routine compute cos(x) for  2^-27 < |x|< 0.25 by  Taylor with more   */
/* precision  and if still doesn't accurate enough by mpcos   or docos  */
/************************************************************************/

static double
SECTION
cslow2 (double x)
{
  mynumber u;
  double w[2], y, cor, res;

  y = ABS (x);
  u.x = big + y;
  y = y - (u.x - big);
  res = do_cos_slow (u, y, 0, 0, &cor);
  if (res == res + cor)
    return res;
  else
    {
      y = ABS (x);
      __docos (y, 0, w);
      if (w[0] == w[0] + 1.000000005 * w[1])
	return w[0];
      else
	return __mpcos (x, 0, false);
    }
}

/***************************************************************************/
/*  Routine compute cos(x+dx) (Double-Length number) where x is small enough*/
/* to use Taylor series around zero and   (x+dx) .Routine receive also      */
/* (right argument) the  original   value of x for computing error of      */
/* result.And if result not accurate enough routine calls other routines    */
/***************************************************************************/


static double
SECTION
csloww (double x, double dx, double orig)
{
  double y, t, res, cor, w[2], a, da, xn;
  mynumber v;
  int4 n;

  /* Taylor series */
  res = TAYLOR_SLOW (x, dx, cor);

  if (cor > 0)
    cor = 1.0005 * cor + ABS (orig) * 3.1e-30;
  else
    cor = 1.0005 * cor - ABS (orig) * 3.1e-30;

  if (res == res + cor)
    return res;
  else
    {
      (x > 0) ? __dubsin (x, dx, w) : __dubsin (-x, -dx, w);

      if (w[1] > 0)
	cor = 1.000000001 * w[1] + ABS (orig) * 1.1e-30;
      else
	cor = 1.000000001 * w[1] - ABS (orig) * 1.1e-30;

      if (w[0] == w[0] + cor)
	return (x > 0) ? w[0] : -w[0];
      else
	{
	  t = (orig * hpinv + toint);
	  xn = t - toint;
	  v.x = t;
	  y = (orig - xn * mp1) - xn * mp2;
	  n = v.i[LOW_HALF] & 3;
	  da = xn * pp3;
	  t = y - da;
	  da = (y - t) - da;
	  y = xn * pp4;
	  a = t - y;
	  da = ((t - a) - y) + da;
	  if (n == 1)
	    {
	      a = -a;
	      da = -da;
	    }
	  (a > 0) ? __dubsin (a, da, w) : __dubsin (-a, -da, w);

	  if (w[1] > 0)
	    cor = 1.000000001 * w[1] + ABS (orig) * 1.1e-40;
	  else
	    cor = 1.000000001 * w[1] - ABS (orig) * 1.1e-40;

	  if (w[0] == w[0] + cor)
	    return (a > 0) ? w[0] : -w[0];
	  else
	    return __mpcos (orig, 0, true);
	}
    }
}

/***************************************************************************/
/*  Routine compute sin(x+dx)   (Double-Length number) where x in first or  */
/*  third quarter of unit circle.Routine receive also (right argument) the  */
/*  original   value of x for computing error of result.And if result not  */
/* accurate enough routine calls  other routines                            */
/***************************************************************************/

static double
SECTION
csloww1 (double x, double dx, double orig, int m)
{
  mynumber u;
  double w[2], y, cor, res;

  u.x = big + x;
  y = x - (u.x - big);
  res = do_sin_slow (u, y, dx, 3.1e-30 * ABS (orig), &cor);

  if (res == res + cor)
    return (m > 0) ? res : -res;
  else
    {
      __dubsin (x, dx, w);
      if (w[1] > 0)
	cor = 1.000000005 * w[1] + 1.1e-30 * ABS (orig);
      else
	cor = 1.000000005 * w[1] - 1.1e-30 * ABS (orig);
      if (w[0] == w[0] + cor)
	return (m > 0) ? w[0] : -w[0];
      else
	return __mpcos (orig, 0, true);
    }
}


/***************************************************************************/
/*  Routine compute sin(x+dx)   (Double-Length number) where x in second or */
/*  fourth quarter of unit circle.Routine receive also  the  original value */
/* and quarter(n= 1or 3)of x for computing error of result.And if result not*/
/* accurate enough routine calls  other routines                            */
/***************************************************************************/

static double
SECTION
csloww2 (double x, double dx, double orig, int n)
{
  mynumber u;
  double w[2], y, cor, res;

  u.x = big + x;
  y = x - (u.x - big);
  res = do_cos_slow (u, y, dx, 3.1e-30 * ABS (orig), &cor);

  if (res == res + cor)
    return (n) ? -res : res;
  else
    {
      __docos (x, dx, w);
      if (w[1] > 0)
	cor = 1.000000005 * w[1] + 1.1e-30 * ABS (orig);
      else
	cor = 1.000000005 * w[1] - 1.1e-30 * ABS (orig);
      if (w[0] == w[0] + cor)
	return (n) ? -w[0] : w[0];
      else
	return __mpcos (orig, 0, true);
    }
}

#ifndef __cos
weak_alias (__cos, cos)
# ifdef NO_LONG_DOUBLE
strong_alias (__cos, __cosl)
weak_alias (__cos, cosl)
# endif
#endif
#ifndef __sin
weak_alias (__sin, sin)
# ifdef NO_LONG_DOUBLE
strong_alias (__sin, __sinl)
weak_alias (__sin, sinl)
# endif
#endif
