/*
 * IBM Accurate Mathematical Library
 * written by International Business Machines Corp.
 * Copyright (C) 2001-2018 Free Software Foundation, Inc.
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
#include <float.h>
#include "endian.h"
#include "mydefs.h"
#include "usncs.h"
#include "MathLib.h"
#include <math.h>
#include <math_private.h>
#include <libm-alias-double.h>
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
static double sloww (double x, double dx, double orig, bool shift_quadrant);
static double sloww1 (double x, double dx, double orig, bool shift_quadrant);
static double sloww2 (double x, double dx, double orig, int n);
static double bsloww (double x, double dx, double orig, int n);
static double bsloww1 (double x, double dx, double orig, int n);
static double bsloww2 (double x, double dx, double orig, int n);
int __branred (double x, double *a, double *aa);
static double cslow2 (double x);

/* Given a number partitioned into X and DX, this function computes the cosine
   of the number by combining the sin and cos of X (as computed by a variation
   of the Taylor series) with the values looked up from the sin/cos table to
   get the result in RES and a correction value in COR.  */
static inline double
__always_inline
do_cos (double x, double dx, double *corp)
{
  mynumber u;

  if (x < 0)
    dx = -dx;

  u.x = big + fabs (x);
  x = fabs (x) - (u.x - big) + dx;

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

/* A more precise variant of DO_COS.  EPS is the adjustment to the correction
   COR.  */
static inline double
__always_inline
do_cos_slow (double x, double dx, double eps, double *corp)
{
  mynumber u;

  if (x <= 0)
    dx = -dx;

  u.x = big + fabs (x);
  x = fabs (x) - (u.x - big);

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
  cor = 1.0005 * cor + __copysign (eps, cor);
  *corp = cor;
  return res;
}

/* Given a number partitioned into X and DX, this function computes the sine of
   the number by combining the sin and cos of X (as computed by a variation of
   the Taylor series) with the values looked up from the sin/cos table to get
   the result in RES and a correction value in COR.  */
static inline double
__always_inline
do_sin (double x, double dx, double *corp)
{
  mynumber u;

  if (x <= 0)
    dx = -dx;
  u.x = big + fabs (x);
  x = fabs (x) - (u.x - big);

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

/* A more precise variant of DO_SIN.  EPS is the adjustment to the correction
   COR.  */
static inline double
__always_inline
do_sin_slow (double x, double dx, double eps, double *corp)
{
  mynumber u;

  if (x <= 0)
    dx = -dx;
  u.x = big + fabs (x);
  x = fabs (x) - (u.x - big);

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
  cor = 1.0005 * cor + __copysign (eps, cor);
  *corp = cor;
  return res;
}

/* Reduce range of X and compute sin of a + da. When SHIFT_QUADRANT is true,
   the routine returns the cosine of a + da by rotating the quadrant once and
   computing the sine of the result.  */
static inline double
__always_inline
reduce_and_compute (double x, bool shift_quadrant)
{
  double retval = 0, a, da;
  unsigned int n = __branred (x, &a, &da);
  int4 k = (n + shift_quadrant) % 4;
  switch (k)
    {
    case 2:
      a = -a;
      da = -da;
      /* Fall through.  */
    case 0:
      if (a * a < 0.01588)
	retval = bsloww (a, da, x, n);
      else
	retval = bsloww1 (a, da, x, n);
      break;

    case 1:
    case 3:
      retval = bsloww2 (a, da, x, n);
      break;
    }
  return retval;
}

static inline int4
__always_inline
reduce_sincos_1 (double x, double *a, double *da)
{
  mynumber v;

  double t = (x * hpinv + toint);
  double xn = t - toint;
  v.x = t;
  double y = (x - xn * mp1) - xn * mp2;
  int4 n = v.i[LOW_HALF] & 3;
  double db = xn * mp3;
  double b = y - db;
  db = (y - b) - db;

  *a = b;
  *da = db;

  return n;
}

/* Compute sin (A + DA).  cos can be computed by passing SHIFT_QUADRANT as
   true, which results in shifting the quadrant N clockwise.  */
static double
__always_inline
do_sincos_1 (double a, double da, double x, int4 n, bool shift_quadrant)
{
  double xx, retval, res, cor;
  double eps = fabs (x) * 1.2e-30;

  int k1 = (n + shift_quadrant) & 3;
  switch (k1)
    {			/* quarter of unit circle */
    case 2:
      a = -a;
      da = -da;
      /* Fall through.  */
    case 0:
      xx = a * a;
      if (xx < 0.01588)
	{
	  /* Taylor series.  */
	  res = TAYLOR_SIN (xx, a, da, cor);
	  cor = 1.02 * cor + __copysign (eps, cor);
	  retval = (res == res + cor) ? res : sloww (a, da, x, shift_quadrant);
	}
      else
	{
	  res = do_sin (a, da, &cor);
	  cor = 1.035 * cor + __copysign (eps, cor);
	  retval = ((res == res + cor) ? __copysign (res, a)
		    : sloww1 (a, da, x, shift_quadrant));
	}
      break;

    case 1:
    case 3:
      res = do_cos (a, da, &cor);
      cor = 1.025 * cor + __copysign (eps, cor);
      retval = ((res == res + cor) ? ((n & 2) ? -res : res)
		: sloww2 (a, da, x, n));
      break;
    }

  return retval;
}

static inline int4
__always_inline
reduce_sincos_2 (double x, double *a, double *da)
{
  mynumber v;

  double t = (x * hpinv + toint);
  double xn = t - toint;
  v.x = t;
  double xn1 = (xn + 8.0e22) - 8.0e22;
  double xn2 = xn - xn1;
  double y = ((((x - xn1 * mp1) - xn1 * mp2) - xn2 * mp1) - xn2 * mp2);
  int4 n = v.i[LOW_HALF] & 3;
  double db = xn1 * pp3;
  t = y - db;
  db = (y - t) - db;
  db = (db - xn2 * pp3) - xn * pp4;
  double b = t + db;
  db = (t - b) + db;

  *a = b;
  *da = db;

  return n;
}

/* Compute sin (A + DA).  cos can be computed by passing SHIFT_QUADRANT as
   true, which results in shifting the quadrant N clockwise.  */
static double
__always_inline
do_sincos_2 (double a, double da, double x, int4 n, bool shift_quadrant)
{
  double res, retval, cor, xx;

  double eps = 1.0e-24;

  int4 k = (n + shift_quadrant) & 3;

  switch (k)
    {
    case 2:
      a = -a;
      da = -da;
      /* Fall through.  */
    case 0:
      xx = a * a;
      if (xx < 0.01588)
	{
	  /* Taylor series.  */
	  res = TAYLOR_SIN (xx, a, da, cor);
	  cor = 1.02 * cor + __copysign (eps, cor);
	  retval = (res == res + cor) ? res : bsloww (a, da, x, n);
	}
      else
	{
	  res = do_sin (a, da, &cor);
	  cor = 1.035 * cor + __copysign (eps, cor);
	  retval = ((res == res + cor) ? __copysign (res, a)
		    : bsloww1 (a, da, x, n));
	}
      break;

    case 1:
    case 3:
      res = do_cos (a, da, &cor);
      cor = 1.025 * cor + __copysign (eps, cor);
      retval = ((res == res + cor) ? ((n & 2) ? -res : res)
		: bsloww2 (a, da, x, n));
      break;
    }

  return retval;
}

/*******************************************************************/
/* An ultimate sin routine. Given an IEEE double machine number x   */
/* it computes the correctly rounded (to nearest) value of sin(x)  */
/*******************************************************************/
#ifdef IN_SINCOS
static double
#else
double
SECTION
#endif
__sin (double x)
{
  double xx, res, t, cor;
  mynumber u;
  int4 k, m;
  double retval = 0;

#ifndef IN_SINCOS
  SET_RESTORE_ROUND_53BIT (FE_TONEAREST);
#endif

  u.x = x;
  m = u.i[HIGH_HALF];
  k = 0x7fffffff & m;		/* no sign           */
  if (k < 0x3e500000)		/* if x->0 =>sin(x)=x */
    {
      math_check_force_underflow (x);
      retval = x;
    }
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
      res = do_sin (x, 0, &cor);
      retval = (res == res + 1.096 * cor) ? res : slow1 (x);
      retval = __copysign (retval, x);
    }				/*   else  if (k < 0x3feb6000)    */

/*----------------------- 0.855469  <|x|<2.426265  ----------------------*/
  else if (k < 0x400368fd)
    {

      t = hp0 - fabs (x);
      res = do_cos (t, hp1, &cor);
      retval = (res == res + 1.020 * cor) ? res : slow2 (x);
      retval = __copysign (retval, x);
    }				/*   else  if (k < 0x400368fd)    */

#ifndef IN_SINCOS
/*-------------------------- 2.426265<|x|< 105414350 ----------------------*/
  else if (k < 0x419921FB)
    {
      double a, da;
      int4 n = reduce_sincos_1 (x, &a, &da);
      retval = do_sincos_1 (a, da, x, n, false);
    }				/*   else  if (k <  0x419921FB )    */

/*---------------------105414350 <|x|< 281474976710656 --------------------*/
  else if (k < 0x42F00000)
    {
      double a, da;

      int4 n = reduce_sincos_2 (x, &a, &da);
      retval = do_sincos_2 (a, da, x, n, false);
    }				/*   else  if (k <  0x42F00000 )   */

/* -----------------281474976710656 <|x| <2^1024----------------------------*/
  else if (k < 0x7ff00000)
    retval = reduce_and_compute (x, false);

/*--------------------- |x| > 2^1024 ----------------------------------*/
  else
    {
      if (k == 0x7ff00000 && u.i[LOW_HALF] == 0)
	__set_errno (EDOM);
      retval = x / x;
    }
#endif

  return retval;
}


/*******************************************************************/
/* An ultimate cos routine. Given an IEEE double machine number x   */
/* it computes the correctly rounded (to nearest) value of cos(x)  */
/*******************************************************************/

#ifdef IN_SINCOS
static double
#else
double
SECTION
#endif
__cos (double x)
{
  double y, xx, res, cor, a, da;
  mynumber u;
  int4 k, m;

  double retval = 0;

#ifndef IN_SINCOS
  SET_RESTORE_ROUND_53BIT (FE_TONEAREST);
#endif

  u.x = x;
  m = u.i[HIGH_HALF];
  k = 0x7fffffff & m;

  /* |x|<2^-27 => cos(x)=1 */
  if (k < 0x3e400000)
    retval = 1.0;

  else if (k < 0x3feb6000)
    {				/* 2^-27 < |x| < 0.855469 */
      res = do_cos (x, 0, &cor);
      retval = (res == res + 1.020 * cor) ? res : cslow2 (x);
    }				/*   else  if (k < 0x3feb6000)    */

  else if (k < 0x400368fd)
    { /* 0.855469  <|x|<2.426265  */ ;
      y = hp0 - fabs (x);
      a = y + hp1;
      da = (y - a) + hp1;
      xx = a * a;
      if (xx < 0.01588)
	{
	  res = TAYLOR_SIN (xx, a, da, cor);
	  cor = 1.02 * cor + __copysign (1.0e-31, cor);
	  retval = (res == res + cor) ? res : sloww (a, da, x, true);
	}
      else
	{
	  res = do_sin (a, da, &cor);
	  cor = 1.035 * cor + __copysign (1.0e-31, cor);
	  retval = ((res == res + cor) ? __copysign (res, a)
		    : sloww1 (a, da, x, true));
	}

    }				/*   else  if (k < 0x400368fd)    */


#ifndef IN_SINCOS
  else if (k < 0x419921FB)
    {				/* 2.426265<|x|< 105414350 */
      double a, da;
      int4 n = reduce_sincos_1 (x, &a, &da);
      retval = do_sincos_1 (a, da, x, n, true);
    }				/*   else  if (k <  0x419921FB )    */

  else if (k < 0x42F00000)
    {
      double a, da;

      int4 n = reduce_sincos_2 (x, &a, &da);
      retval = do_sincos_2 (a, da, x, n, true);
    }				/*   else  if (k <  0x42F00000 )    */

  /* 281474976710656 <|x| <2^1024 */
  else if (k < 0x7ff00000)
    retval = reduce_and_compute (x, true);

  else
    {
      if (k == 0x7ff00000 && u.i[LOW_HALF] == 0)
	__set_errno (EDOM);
      retval = x / x;		/* |x| > 2^1024 */
    }
#endif

  return retval;
}

/************************************************************************/
/*  Routine compute sin(x) for  2^-26 < |x|< 0.25 by  Taylor with more   */
/* precision  and if still doesn't accurate enough by mpsin   or dubsin */
/************************************************************************/

static inline double
__always_inline
slow (double x)
{
  double res, cor, w[2];
  res = TAYLOR_SLOW (x, 0, cor);
  if (res == res + 1.0007 * cor)
    return res;

  __dubsin (fabs (x), 0, w);
  if (w[0] == w[0] + 1.000000001 * w[1])
    return __copysign (w[0], x);

  return __copysign (__mpsin (fabs (x), 0, false), x);
}

/*******************************************************************************/
/* Routine compute sin(x) for 0.25<|x|< 0.855469 by __sincostab.tbl and Taylor */
/* and if result still doesn't accurate enough by mpsin   or dubsin            */
/*******************************************************************************/

static inline double
__always_inline
slow1 (double x)
{
  double w[2], cor, res;

  res = do_sin_slow (x, 0, 0, &cor);
  if (res == res + cor)
    return res;

  __dubsin (fabs (x), 0, w);
  if (w[0] == w[0] + 1.000000005 * w[1])
    return w[0];

  return __mpsin (fabs (x), 0, false);
}

/**************************************************************************/
/*  Routine compute sin(x) for   0.855469  <|x|<2.426265  by  __sincostab.tbl  */
/* and if result still doesn't accurate enough by mpsin   or dubsin       */
/**************************************************************************/
static inline double
__always_inline
slow2 (double x)
{
  double w[2], y, y1, y2, cor, res;

  double t = hp0 - fabs (x);
  res = do_cos_slow (t, hp1, 0, &cor);
  if (res == res + cor)
    return res;

  y = fabs (x) - hp0;
  y1 = y - hp1;
  y2 = (y - y1) - hp1;
  __docos (y1, y2, w);
  if (w[0] == w[0] + 1.000000005 * w[1])
    return w[0];

  return __mpsin (fabs (x), 0, false);
}

/* Compute sin(x + dx) where X is small enough to use Taylor series around zero
   and (x + dx) in the first or third quarter of the unit circle.  ORIG is the
   original value of X for computing error of the result.  If the result is not
   accurate enough, the routine calls mpsin or dubsin.  SHIFT_QUADRANT rotates
   the unit circle by 1 to compute the cosine instead of sine.  */
static inline double
__always_inline
sloww (double x, double dx, double orig, bool shift_quadrant)
{
  double y, t, res, cor, w[2], a, da, xn;
  mynumber v;
  int4 n;
  res = TAYLOR_SLOW (x, dx, cor);

  double eps = fabs (orig) * 3.1e-30;

  cor = 1.0005 * cor + __copysign (eps, cor);

  if (res == res + cor)
    return res;

  a = fabs (x);
  da = (x > 0) ? dx : -dx;
  __dubsin (a, da, w);
  eps = fabs (orig) * 1.1e-30;
  cor = 1.000000001 * w[1] + __copysign (eps, w[1]);

  if (w[0] == w[0] + cor)
    return __copysign (w[0], x);

  t = (orig * hpinv + toint);
  xn = t - toint;
  v.x = t;
  y = (orig - xn * mp1) - xn * mp2;
  n = (v.i[LOW_HALF] + shift_quadrant) & 3;
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
  x = fabs (a);
  dx = (a > 0) ? da : -da;
  __dubsin (x, dx, w);
  eps = fabs (orig) * 1.1e-40;
  cor = 1.000000001 * w[1] + __copysign (eps, w[1]);

  if (w[0] == w[0] + cor)
    return __copysign (w[0], a);

  return shift_quadrant ? __mpcos (orig, 0, true) : __mpsin (orig, 0, true);
}

/* Compute sin(x + dx) where X is in the first or third quarter of the unit
   circle.  ORIG is the original value of X for computing error of the result.
   If the result is not accurate enough, the routine calls mpsin or dubsin.
   SHIFT_QUADRANT rotates the unit circle by 1 to compute the cosine instead of
   sine.  */
static inline double
__always_inline
sloww1 (double x, double dx, double orig, bool shift_quadrant)
{
  double w[2], cor, res;

  res = do_sin_slow (x, dx, 3.1e-30 * fabs (orig), &cor);

  if (res == res + cor)
    return __copysign (res, x);

  dx = (x > 0 ? dx : -dx);
  __dubsin (fabs (x), dx, w);

  double eps = 1.1e-30 * fabs (orig);
  cor = 1.000000005 * w[1] + __copysign (eps, w[1]);

  if (w[0] == w[0] + cor)
    return __copysign (w[0], x);

  return shift_quadrant ? __mpcos (orig, 0, true) : __mpsin (orig, 0, true);
}

/***************************************************************************/
/*  Routine compute sin(x+dx)   (Double-Length number) where x in second or */
/*  fourth quarter of unit circle.Routine receive also  the  original value */
/* and quarter(n= 1or 3)of x for computing error of result.And if result not*/
/* accurate enough routine calls  mpsin1   or dubsin                       */
/***************************************************************************/

static inline double
__always_inline
sloww2 (double x, double dx, double orig, int n)
{
  double w[2], cor, res;

  res = do_cos_slow (x, dx, 3.1e-30 * fabs (orig), &cor);

  if (res == res + cor)
    return (n & 2) ? -res : res;

  dx = x > 0 ? dx : -dx;
  __docos (fabs (x), dx, w);

  double eps = 1.1e-30 * fabs (orig);
  cor = 1.000000005 * w[1] + __copysign (eps, w[1]);

  if (w[0] == w[0] + cor)
    return (n & 2) ? -w[0] : w[0];

  return (n & 1) ? __mpsin (orig, 0, true) : __mpcos (orig, 0, true);
}

/***************************************************************************/
/*  Routine compute sin(x+dx) or cos(x+dx) (Double-Length number) where x   */
/* is small enough to use Taylor series around zero and   (x+dx)            */
/* in first or third quarter of unit circle.Routine receive also            */
/* (right argument) the  original   value of x for computing error of      */
/* result.And if result not accurate enough routine calls other routines    */
/***************************************************************************/

static inline double
__always_inline
bsloww (double x, double dx, double orig, int n)
{
  double res, cor, w[2], a, da;

  res = TAYLOR_SLOW (x, dx, cor);
  cor = 1.0005 * cor + __copysign (1.1e-24, cor);
  if (res == res + cor)
    return res;

  a = fabs (x);
  da = (x > 0) ? dx : -dx;
  __dubsin (a, da, w);
  cor = 1.000000001 * w[1] + __copysign (1.1e-24, w[1]);

  if (w[0] == w[0] + cor)
    return __copysign (w[0], x);

  return (n & 1) ? __mpcos (orig, 0, true) : __mpsin (orig, 0, true);
}

/***************************************************************************/
/*  Routine compute sin(x+dx)  or cos(x+dx) (Double-Length number) where x  */
/* in first or third quarter of unit circle.Routine receive also            */
/* (right argument) the original  value of x for computing error of result.*/
/* And if result not  accurate enough routine calls  other routines         */
/***************************************************************************/

static inline double
__always_inline
bsloww1 (double x, double dx, double orig, int n)
{
  double w[2], cor, res;

  res = do_sin_slow (x, dx, 1.1e-24, &cor);
  if (res == res + cor)
    return (x > 0) ? res : -res;

  dx = (x > 0) ? dx : -dx;
  __dubsin (fabs (x), dx, w);

  cor = 1.000000005 * w[1] + __copysign (1.1e-24, w[1]);

  if (w[0] == w[0] + cor)
    return __copysign (w[0], x);

  return (n & 1) ? __mpcos (orig, 0, true) : __mpsin (orig, 0, true);
}

/***************************************************************************/
/*  Routine compute sin(x+dx)  or cos(x+dx) (Double-Length number) where x  */
/* in second or fourth quarter of unit circle.Routine receive also  the     */
/* original value and quarter(n= 1or 3)of x for computing error of result.  */
/* And if result not accurate enough routine calls  other routines          */
/***************************************************************************/

static inline double
__always_inline
bsloww2 (double x, double dx, double orig, int n)
{
  double w[2], cor, res;

  res = do_cos_slow (x, dx, 1.1e-24, &cor);
  if (res == res + cor)
    return (n & 2) ? -res : res;

  dx = (x > 0) ? dx : -dx;
  __docos (fabs (x), dx, w);

  cor = 1.000000005 * w[1] + __copysign (1.1e-24, w[1]);

  if (w[0] == w[0] + cor)
    return (n & 2) ? -w[0] : w[0];

  return (n & 1) ? __mpsin (orig, 0, true) : __mpcos (orig, 0, true);
}

/************************************************************************/
/*  Routine compute cos(x) for  2^-27 < |x|< 0.25 by  Taylor with more   */
/* precision  and if still doesn't accurate enough by mpcos   or docos  */
/************************************************************************/

static inline double
__always_inline
cslow2 (double x)
{
  double w[2], cor, res;

  res = do_cos_slow (x, 0, 0, &cor);
  if (res == res + cor)
    return res;

  __docos (fabs (x), 0, w);
  if (w[0] == w[0] + 1.000000005 * w[1])
    return w[0];

  return __mpcos (x, 0, false);
}

#ifndef __cos
libm_alias_double (__cos, cos)
#endif
#ifndef __sin
libm_alias_double (__sin, sin)
#endif
