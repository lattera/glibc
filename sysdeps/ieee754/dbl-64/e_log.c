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
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
/*********************************************************************/
/*                                                                   */
/*      MODULE_NAME:ulog.c                                           */
/*                                                                   */
/*      FUNCTION:ulog                                                */
/*                                                                   */
/*      FILES NEEDED: dla.h endian.h mpa.h mydefs.h ulog.h           */
/*                    ulog.tbl                                       */
/*                                                                   */
/* An ultimate log routine. Given an IEEE double machine number x    */
/* it computes the rounded (to nearest) value of log(x).	     */
/* Assumption: Machine arithmetic operations are performed in        */
/* round to nearest mode of IEEE 754 standard.                       */
/*                                                                   */
/*********************************************************************/


#include "endian.h"
#include <dla.h>
#include "mpa.h"
#include "MathLib.h"
#include <math.h>
#include <math_private.h>

#ifndef SECTION
# define SECTION
#endif

/*********************************************************************/
/* An ultimate log routine. Given an IEEE double machine number x    */
/* it computes the rounded (to nearest) value of log(x).	     */
/*********************************************************************/
double
SECTION
__ieee754_log (double x)
{
  int i, j, n, ux, dx;
  double dbl_n, u, p0, q, r0, w, nln2a, luai, lubi, lvaj, lvbj,
	 sij, ssij, ttij, A, B, B0, polI, polII, t8, a, aa, b, bb, c;
#ifndef DLA_FMS
  double t1, t2, t3, t4, t5;
#endif
  number num;

#include "ulog.tbl"
#include "ulog.h"

  /* Treating special values of x ( x<=0, x=INF, x=NaN etc.). */

  num.d = x;
  ux = num.i[HIGH_HALF];
  dx = num.i[LOW_HALF];
  n = 0;
  if (__glibc_unlikely (ux < 0x00100000))
    {
      if (__glibc_unlikely (((ux & 0x7fffffff) | dx) == 0))
	return MHALF / 0.0;     /* return -INF */
      if (__glibc_unlikely (ux < 0))
	return (x - x) / 0.0;   /* return NaN  */
      n -= 54;
      x *= two54.d;             /* scale x     */
      num.d = x;
    }
  if (__glibc_unlikely (ux >= 0x7ff00000))
    return x + x;               /* INF or NaN  */

  /* Regular values of x */

  w = x - 1;
  if (__glibc_likely (fabs (w) > U03))
    goto case_03;

  /* log (1) is +0 in all rounding modes.  */
  if (w == 0.0)
    return 0.0;

  /*--- The case abs(x-1) < 0.03 */

  t8 = MHALF * w;
  EMULV (t8, w, a, aa, t1, t2, t3, t4, t5);
  EADD (w, a, b, bb);
  /* Evaluate polynomial II */
  polII = b7.d + w * b8.d;
  polII = b6.d + w * polII;
  polII = b5.d + w * polII;
  polII = b4.d + w * polII;
  polII = b3.d + w * polII;
  polII = b2.d + w * polII;
  polII = b1.d + w * polII;
  polII = b0.d + w * polII;
  polII *= w * w * w;
  c = (aa + bb) + polII;

  /* Here b contains the high part of the result, and c the low part.
     Maximum error is b * 2.334e-19, so accuracy is >61 bits.
     Therefore max ULP error of b + c is ~0.502.  */
  return b + c;

  /*--- The case abs(x-1) > 0.03 */
case_03:

  /* Find n,u such that x = u*2**n,   1/sqrt(2) < u < sqrt(2)  */
  n += (num.i[HIGH_HALF] >> 20) - 1023;
  num.i[HIGH_HALF] = (num.i[HIGH_HALF] & 0x000fffff) | 0x3ff00000;
  if (num.d > SQRT_2)
    {
      num.d *= HALF;
      n++;
    }
  u = num.d;
  dbl_n = (double) n;

  /* Find i such that ui=1+(i-75)/2**8 is closest to u (i= 0,1,2,...,181) */
  num.d += h1.d;
  i = (num.i[HIGH_HALF] & 0x000fffff) >> 12;

  /* Find j such that vj=1+(j-180)/2**16 is closest to v=u/ui (j= 0,...,361) */
  num.d = u * Iu[i].d + h2.d;
  j = (num.i[HIGH_HALF] & 0x000fffff) >> 4;

  /* Compute w=(u-ui*vj)/(ui*vj) */
  p0 = (1 + (i - 75) * DEL_U) * (1 + (j - 180) * DEL_V);
  q = u - p0;
  r0 = Iu[i].d * Iv[j].d;
  w = q * r0;

  /* Evaluate polynomial I */
  polI = w + (a2.d + a3.d * w) * w * w;

  /* Add up everything */
  nln2a = dbl_n * LN2A;
  luai = Lu[i][0].d;
  lubi = Lu[i][1].d;
  lvaj = Lv[j][0].d;
  lvbj = Lv[j][1].d;
  EADD (luai, lvaj, sij, ssij);
  EADD (nln2a, sij, A, ttij);
  B0 = (((lubi + lvbj) + ssij) + ttij) + dbl_n * LN2B;
  B = polI + B0;

  /* Here A contains the high part of the result, and B the low part.
     Maximum abs error is 6.095e-21 and min log (x) is 0.0295 since x > 1.03.
     Therefore max ULP error of A + B is ~0.502.  */
  return A + B;
}

#ifndef __ieee754_log
strong_alias (__ieee754_log, __log_finite)
#endif
