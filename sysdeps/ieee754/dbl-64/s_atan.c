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
/************************************************************************/
/*  MODULE_NAME: atnat.c                                                */
/*                                                                      */
/*  FUNCTIONS:  uatan                                                   */
/*              atanMp                                                  */
/*              signArctan                                              */
/*                                                                      */
/*                                                                      */
/*  FILES NEEDED: dla.h endian.h mpa.h mydefs.h atnat.h                 */
/*                mpatan.c mpatan2.c mpsqrt.c                           */
/*                uatan.tbl                                             */
/*                                                                      */
/* An ultimate atan() routine. Given an IEEE double machine number x    */
/* it computes the correctly rounded (to nearest) value of atan(x).     */
/*                                                                      */
/* Assumption: Machine arithmetic operations are performed in           */
/* round to nearest mode of IEEE 754 standard.                          */
/*                                                                      */
/************************************************************************/

#include <dla.h>
#include "mpa.h"
#include "MathLib.h"
#include "uatan.tbl"
#include "atnat.h"
#include <fenv.h>
#include <float.h>
#include <libm-alias-double.h>
#include <math.h>
#include <math_private.h>
#include <stap-probe.h>

void __mpatan (mp_no *, mp_no *, int);	/* see definition in mpatan.c */
static double atanMp (double, const int[]);

  /* Fix the sign of y and return */
static double
__signArctan (double x, double y)
{
  return __copysign (y, x);
}


/* An ultimate atan() routine. Given an IEEE double machine number x,    */
/* routine computes the correctly rounded (to nearest) value of atan(x). */
double
__atan (double x)
{
  double cor, s1, ss1, s2, ss2, t1, t2, t3, t7, t8, t9, t10, u, u2, u3,
	 v, vv, w, ww, y, yy, z, zz;
#ifndef DLA_FMS
  double t4, t5, t6;
#endif
  int i, ux, dx;
  static const int pr[M] = { 6, 8, 10, 32 };
  number num;

  num.d = x;
  ux = num.i[HIGH_HALF];
  dx = num.i[LOW_HALF];

  /* x=NaN */
  if (((ux & 0x7ff00000) == 0x7ff00000)
      && (((ux & 0x000fffff) | dx) != 0x00000000))
    return x + x;

  /* Regular values of x, including denormals +-0 and +-INF */
  SET_RESTORE_ROUND (FE_TONEAREST);
  u = (x < 0) ? -x : x;
  if (u < C)
    {
      if (u < B)
	{
	  if (u < A)
	    {
	      math_check_force_underflow_nonneg (u);
	      return x;
	    }
	  else
	    {			/* A <= u < B */
	      v = x * x;
	      yy = d11.d + v * d13.d;
	      yy = d9.d + v * yy;
	      yy = d7.d + v * yy;
	      yy = d5.d + v * yy;
	      yy = d3.d + v * yy;
	      yy *= x * v;

	      if ((y = x + (yy - U1 * x)) == x + (yy + U1 * x))
		return y;

	      EMULV (x, x, v, vv, t1, t2, t3, t4, t5);	/* v+vv=x^2 */

	      s1 = f17.d + v * f19.d;
	      s1 = f15.d + v * s1;
	      s1 = f13.d + v * s1;
	      s1 = f11.d + v * s1;
	      s1 *= v;

	      ADD2 (f9.d, ff9.d, s1, 0, s2, ss2, t1, t2);
	      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	      ADD2 (f7.d, ff7.d, s1, ss1, s2, ss2, t1, t2);
	      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	      ADD2 (f5.d, ff5.d, s1, ss1, s2, ss2, t1, t2);
	      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	      ADD2 (f3.d, ff3.d, s1, ss1, s2, ss2, t1, t2);
	      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	      MUL2 (x, 0, s1, ss1, s2, ss2, t1, t2, t3, t4, t5, t6, t7,
		    t8);
	      ADD2 (x, 0, s2, ss2, s1, ss1, t1, t2);
	      if ((y = s1 + (ss1 - U5 * s1)) == s1 + (ss1 + U5 * s1))
		return y;

	      return atanMp (x, pr);
	    }
	}
      else
	{			/* B <= u < C */
	  i = (TWO52 + TWO8 * u) - TWO52;
	  i -= 16;
	  z = u - cij[i][0].d;
	  yy = cij[i][5].d + z * cij[i][6].d;
	  yy = cij[i][4].d + z * yy;
	  yy = cij[i][3].d + z * yy;
	  yy = cij[i][2].d + z * yy;
	  yy *= z;

	  t1 = cij[i][1].d;
	  if (i < 112)
	    {
	      if (i < 48)
		u2 = U21;	/* u < 1/4        */
	      else
		u2 = U22;
	    }			/* 1/4 <= u < 1/2 */
	  else
	    {
	      if (i < 176)
		u2 = U23;	/* 1/2 <= u < 3/4 */
	      else
		u2 = U24;
	    }			/* 3/4 <= u <= 1  */
	  if ((y = t1 + (yy - u2 * t1)) == t1 + (yy + u2 * t1))
	    return __signArctan (x, y);

	  z = u - hij[i][0].d;

	  s1 = hij[i][14].d + z * hij[i][15].d;
	  s1 = hij[i][13].d + z * s1;
	  s1 = hij[i][12].d + z * s1;
	  s1 = hij[i][11].d + z * s1;
	  s1 *= z;

	  ADD2 (hij[i][9].d, hij[i][10].d, s1, 0, s2, ss2, t1, t2);
	  MUL2 (z, 0, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (hij[i][7].d, hij[i][8].d, s1, ss1, s2, ss2, t1, t2);
	  MUL2 (z, 0, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (hij[i][5].d, hij[i][6].d, s1, ss1, s2, ss2, t1, t2);
	  MUL2 (z, 0, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (hij[i][3].d, hij[i][4].d, s1, ss1, s2, ss2, t1, t2);
	  MUL2 (z, 0, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (hij[i][1].d, hij[i][2].d, s1, ss1, s2, ss2, t1, t2);
	  if ((y = s2 + (ss2 - U6 * s2)) == s2 + (ss2 + U6 * s2))
	    return __signArctan (x, y);

	  return atanMp (x, pr);
	}
    }
  else
    {
      if (u < D)
	{			/* C <= u < D */
	  w = 1 / u;
	  EMULV (w, u, t1, t2, t3, t4, t5, t6, t7);
	  ww = w * ((1 - t1) - t2);
	  i = (TWO52 + TWO8 * w) - TWO52;
	  i -= 16;
	  z = (w - cij[i][0].d) + ww;

	  yy = cij[i][5].d + z * cij[i][6].d;
	  yy = cij[i][4].d + z * yy;
	  yy = cij[i][3].d + z * yy;
	  yy = cij[i][2].d + z * yy;
	  yy = HPI1 - z * yy;

	  t1 = HPI - cij[i][1].d;
	  if (i < 112)
	    u3 = U31;           /* w <  1/2 */
	  else
	    u3 = U32;           /* w >= 1/2 */
	  if ((y = t1 + (yy - u3)) == t1 + (yy + u3))
	    return __signArctan (x, y);

	  DIV2 (1, 0, u, 0, w, ww, t1, t2, t3, t4, t5, t6, t7, t8, t9,
		t10);
	  t1 = w - hij[i][0].d;
	  EADD (t1, ww, z, zz);

	  s1 = hij[i][14].d + z * hij[i][15].d;
	  s1 = hij[i][13].d + z * s1;
	  s1 = hij[i][12].d + z * s1;
	  s1 = hij[i][11].d + z * s1;
	  s1 *= z;

	  ADD2 (hij[i][9].d, hij[i][10].d, s1, 0, s2, ss2, t1, t2);
	  MUL2 (z, zz, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (hij[i][7].d, hij[i][8].d, s1, ss1, s2, ss2, t1, t2);
	  MUL2 (z, zz, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (hij[i][5].d, hij[i][6].d, s1, ss1, s2, ss2, t1, t2);
	  MUL2 (z, zz, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (hij[i][3].d, hij[i][4].d, s1, ss1, s2, ss2, t1, t2);
	  MUL2 (z, zz, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (hij[i][1].d, hij[i][2].d, s1, ss1, s2, ss2, t1, t2);
	  SUB2 (HPI, HPI1, s2, ss2, s1, ss1, t1, t2);
	  if ((y = s1 + (ss1 - U7)) == s1 + (ss1 + U7))
	    return __signArctan (x, y);

	  return atanMp (x, pr);
	}
      else
	{
	  if (u < E)
	    {                   /* D <= u < E */
	      w = 1 / u;
	      v = w * w;
	      EMULV (w, u, t1, t2, t3, t4, t5, t6, t7);

	      yy = d11.d + v * d13.d;
	      yy = d9.d + v * yy;
	      yy = d7.d + v * yy;
	      yy = d5.d + v * yy;
	      yy = d3.d + v * yy;
	      yy *= w * v;

	      ww = w * ((1 - t1) - t2);
	      ESUB (HPI, w, t3, cor);
	      yy = ((HPI1 + cor) - ww) - yy;
	      if ((y = t3 + (yy - U4)) == t3 + (yy + U4))
		return __signArctan (x, y);

	      DIV2 (1, 0, u, 0, w, ww, t1, t2, t3, t4, t5, t6, t7, t8,
		    t9, t10);
	      MUL2 (w, ww, w, ww, v, vv, t1, t2, t3, t4, t5, t6, t7, t8);

	      s1 = f17.d + v * f19.d;
	      s1 = f15.d + v * s1;
	      s1 = f13.d + v * s1;
	      s1 = f11.d + v * s1;
	      s1 *= v;

	      ADD2 (f9.d, ff9.d, s1, 0, s2, ss2, t1, t2);
	      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	      ADD2 (f7.d, ff7.d, s1, ss1, s2, ss2, t1, t2);
	      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	      ADD2 (f5.d, ff5.d, s1, ss1, s2, ss2, t1, t2);
	      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	      ADD2 (f3.d, ff3.d, s1, ss1, s2, ss2, t1, t2);
	      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	      MUL2 (w, ww, s1, ss1, s2, ss2, t1, t2, t3, t4, t5, t6, t7, t8);
	      ADD2 (w, ww, s2, ss2, s1, ss1, t1, t2);
	      SUB2 (HPI, HPI1, s1, ss1, s2, ss2, t1, t2);

	      if ((y = s2 + (ss2 - U8)) == s2 + (ss2 + U8))
		return __signArctan (x, y);

	      return atanMp (x, pr);
	    }
	  else
	    {
	      /* u >= E */
	      if (x > 0)
		return HPI;
	      else
		return MHPI;
	    }
	}
    }
}

 /* Final stages. Compute atan(x) by multiple precision arithmetic */
static double
atanMp (double x, const int pr[])
{
  mp_no mpx, mpy, mpy2, mperr, mpt1, mpy1;
  double y1, y2;
  int i, p;

  for (i = 0; i < M; i++)
    {
      p = pr[i];
      __dbl_mp (x, &mpx, p);
      __mpatan (&mpx, &mpy, p);
      __dbl_mp (u9[i].d, &mpt1, p);
      __mul (&mpy, &mpt1, &mperr, p);
      __add (&mpy, &mperr, &mpy1, p);
      __sub (&mpy, &mperr, &mpy2, p);
      __mp_dbl (&mpy1, &y1, p);
      __mp_dbl (&mpy2, &y2, p);
      if (y1 == y2)
	{
	  LIBC_PROBE (slowatan, 3, &p, &x, &y1);
	  return y1;
	}
    }
  LIBC_PROBE (slowatan_inexact, 3, &p, &x, &y1);
  return y1;			/*if impossible to do exact computing */
}

#ifndef __atan
libm_alias_double (__atan, atan)
#endif
