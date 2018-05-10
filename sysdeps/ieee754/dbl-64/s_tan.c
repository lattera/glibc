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
/*  MODULE_NAME: utan.c                                              */
/*                                                                   */
/*  FUNCTIONS: utan                                                  */
/*             tanMp                                                 */
/*                                                                   */
/*  FILES NEEDED:dla.h endian.h mpa.h mydefs.h utan.h                */
/*               branred.c sincos32.c mptan.c                        */
/*               utan.tbl                                            */
/*                                                                   */
/* An ultimate tan routine. Given an IEEE double machine number x    */
/* it computes the correctly rounded (to nearest) value of tan(x).   */
/* Assumption: Machine arithmetic operations are performed in        */
/* round to nearest mode of IEEE 754 standard.                       */
/*                                                                   */
/*********************************************************************/

#include <errno.h>
#include <float.h>
#include "endian.h"
#include <dla.h>
#include "mpa.h"
#include "MathLib.h"
#include <math.h>
#include <math_private.h>
#include <math-underflow.h>
#include <libm-alias-double.h>
#include <fenv.h>
#include <stap-probe.h>

#ifndef SECTION
# define SECTION
#endif

static double tanMp (double);
void __mptan (double, mp_no *, int);

double
SECTION
__tan (double x)
{
#include "utan.h"
#include "utan.tbl"

  int ux, i, n;
  double a, da, a2, b, db, c, dc, c1, cc1, c2, cc2, c3, cc3, fi, ffi, gi, pz,
	 s, sy, t, t1, t2, t3, t4, t7, t8, t9, t10, w, x2, xn, xx2, y, ya,
         yya, z0, z, zz, z2, zz2;
#ifndef DLA_FMS
  double t5, t6;
#endif
  int p;
  number num, v;
  mp_no mpa, mpt1, mpt2;

  double retval;

  int __branred (double, double *, double *);
  int __mpranred (double, mp_no *, int);

  SET_RESTORE_ROUND_53BIT (FE_TONEAREST);

  /* x=+-INF, x=NaN */
  num.d = x;
  ux = num.i[HIGH_HALF];
  if ((ux & 0x7ff00000) == 0x7ff00000)
    {
      if ((ux & 0x7fffffff) == 0x7ff00000)
	__set_errno (EDOM);
      retval = x - x;
      goto ret;
    }

  w = (x < 0.0) ? -x : x;

  /* (I) The case abs(x) <= 1.259e-8 */
  if (w <= g1.d)
    {
      math_check_force_underflow_nonneg (w);
      retval = x;
      goto ret;
    }

  /* (II) The case 1.259e-8 < abs(x) <= 0.0608 */
  if (w <= g2.d)
    {
      /* First stage */
      x2 = x * x;

      t2 = d9.d + x2 * d11.d;
      t2 = d7.d + x2 * t2;
      t2 = d5.d + x2 * t2;
      t2 = d3.d + x2 * t2;
      t2 *= x * x2;

      if ((y = x + (t2 - u1.d * t2)) == x + (t2 + u1.d * t2))
	{
	  retval = y;
	  goto ret;
	}

      /* Second stage */
      c1 = a25.d + x2 * a27.d;
      c1 = a23.d + x2 * c1;
      c1 = a21.d + x2 * c1;
      c1 = a19.d + x2 * c1;
      c1 = a17.d + x2 * c1;
      c1 = a15.d + x2 * c1;
      c1 *= x2;

      EMULV (x, x, x2, xx2, t1, t2, t3, t4, t5);
      ADD2 (a13.d, aa13.d, c1, 0.0, c2, cc2, t1, t2);
      MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (a11.d, aa11.d, c1, cc1, c2, cc2, t1, t2);
      MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (a9.d, aa9.d, c1, cc1, c2, cc2, t1, t2);
      MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (a7.d, aa7.d, c1, cc1, c2, cc2, t1, t2);
      MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (a5.d, aa5.d, c1, cc1, c2, cc2, t1, t2);
      MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (a3.d, aa3.d, c1, cc1, c2, cc2, t1, t2);
      MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
      MUL2 (x, 0.0, c1, cc1, c2, cc2, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (x, 0.0, c2, cc2, c1, cc1, t1, t2);
      if ((y = c1 + (cc1 - u2.d * c1)) == c1 + (cc1 + u2.d * c1))
	{
	  retval = y;
	  goto ret;
	}
      retval = tanMp (x);
      goto ret;
    }

  /* (III) The case 0.0608 < abs(x) <= 0.787 */
  if (w <= g3.d)
    {
      /* First stage */
      i = ((int) (mfftnhf.d + TWO8 * w));
      z = w - xfg[i][0].d;
      z2 = z * z;
      s = (x < 0.0) ? -1 : 1;
      pz = z + z * z2 * (e0.d + z2 * e1.d);
      fi = xfg[i][1].d;
      gi = xfg[i][2].d;
      t2 = pz * (gi + fi) / (gi - pz);
      if ((y = fi + (t2 - fi * u3.d)) == fi + (t2 + fi * u3.d))
	{
	  retval = (s * y);
	  goto ret;
	}
      t3 = (t2 < 0.0) ? -t2 : t2;
      t4 = fi * ua3.d + t3 * ub3.d;
      if ((y = fi + (t2 - t4)) == fi + (t2 + t4))
	{
	  retval = (s * y);
	  goto ret;
	}

      /* Second stage */
      ffi = xfg[i][3].d;
      c1 = z2 * (a7.d + z2 * (a9.d + z2 * a11.d));
      EMULV (z, z, z2, zz2, t1, t2, t3, t4, t5);
      ADD2 (a5.d, aa5.d, c1, 0.0, c2, cc2, t1, t2);
      MUL2 (z2, zz2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (a3.d, aa3.d, c1, cc1, c2, cc2, t1, t2);
      MUL2 (z2, zz2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
      MUL2 (z, 0.0, c1, cc1, c2, cc2, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (z, 0.0, c2, cc2, c1, cc1, t1, t2);

      ADD2 (fi, ffi, c1, cc1, c2, cc2, t1, t2);
      MUL2 (fi, ffi, c1, cc1, c3, cc3, t1, t2, t3, t4, t5, t6, t7, t8);
      SUB2 (1.0, 0.0, c3, cc3, c1, cc1, t1, t2);
      DIV2 (c2, cc2, c1, cc1, c3, cc3, t1, t2, t3, t4, t5, t6, t7, t8, t9,
	    t10);

      if ((y = c3 + (cc3 - u4.d * c3)) == c3 + (cc3 + u4.d * c3))
	{
	  retval = (s * y);
	  goto ret;
	}
      retval = tanMp (x);
      goto ret;
    }

  /* (---) The case 0.787 < abs(x) <= 25 */
  if (w <= g4.d)
    {
      /* Range reduction by algorithm i */
      t = (x * hpinv.d + toint.d);
      xn = t - toint.d;
      v.d = t;
      t1 = (x - xn * mp1.d) - xn * mp2.d;
      n = v.i[LOW_HALF] & 0x00000001;
      da = xn * mp3.d;
      a = t1 - da;
      da = (t1 - a) - da;
      if (a < 0.0)
	{
	  ya = -a;
	  yya = -da;
	  sy = -1;
	}
      else
	{
	  ya = a;
	  yya = da;
	  sy = 1;
	}

      /* (IV),(V) The case 0.787 < abs(x) <= 25,    abs(y) <= 1e-7 */
      if (ya <= gy1.d)
	{
	  retval = tanMp (x);
	  goto ret;
	}

      /* (VI) The case 0.787 < abs(x) <= 25,    1e-7 < abs(y) <= 0.0608 */
      if (ya <= gy2.d)
	{
	  a2 = a * a;
	  t2 = d9.d + a2 * d11.d;
	  t2 = d7.d + a2 * t2;
	  t2 = d5.d + a2 * t2;
	  t2 = d3.d + a2 * t2;
	  t2 = da + a * a2 * t2;

	  if (n)
	    {
	      /* First stage -cot */
	      EADD (a, t2, b, db);
	      DIV2 (1.0, 0.0, b, db, c, dc, t1, t2, t3, t4, t5, t6, t7, t8,
		    t9, t10);
	      if ((y = c + (dc - u6.d * c)) == c + (dc + u6.d * c))
		{
		  retval = (-y);
		  goto ret;
		}
	    }
	  else
	    {
	      /* First stage tan */
	      if ((y = a + (t2 - u5.d * a)) == a + (t2 + u5.d * a))
		{
		  retval = y;
		  goto ret;
		}
	    }
	  /* Second stage */
	  /* Range reduction by algorithm ii */
	  t = (x * hpinv.d + toint.d);
	  xn = t - toint.d;
	  v.d = t;
	  t1 = (x - xn * mp1.d) - xn * mp2.d;
	  n = v.i[LOW_HALF] & 0x00000001;
	  da = xn * pp3.d;
	  t = t1 - da;
	  da = (t1 - t) - da;
	  t1 = xn * pp4.d;
	  a = t - t1;
	  da = ((t - a) - t1) + da;

	  /* Second stage */
	  EADD (a, da, t1, t2);
	  a = t1;
	  da = t2;
	  MUL2 (a, da, a, da, x2, xx2, t1, t2, t3, t4, t5, t6, t7, t8);

	  c1 = a25.d + x2 * a27.d;
	  c1 = a23.d + x2 * c1;
	  c1 = a21.d + x2 * c1;
	  c1 = a19.d + x2 * c1;
	  c1 = a17.d + x2 * c1;
	  c1 = a15.d + x2 * c1;
	  c1 *= x2;

	  ADD2 (a13.d, aa13.d, c1, 0.0, c2, cc2, t1, t2);
	  MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (a11.d, aa11.d, c1, cc1, c2, cc2, t1, t2);
	  MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (a9.d, aa9.d, c1, cc1, c2, cc2, t1, t2);
	  MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (a7.d, aa7.d, c1, cc1, c2, cc2, t1, t2);
	  MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (a5.d, aa5.d, c1, cc1, c2, cc2, t1, t2);
	  MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (a3.d, aa3.d, c1, cc1, c2, cc2, t1, t2);
	  MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
	  MUL2 (a, da, c1, cc1, c2, cc2, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (a, da, c2, cc2, c1, cc1, t1, t2);

	  if (n)
	    {
	      /* Second stage -cot */
	      DIV2 (1.0, 0.0, c1, cc1, c2, cc2, t1, t2, t3, t4, t5, t6, t7,
		    t8, t9, t10);
	      if ((y = c2 + (cc2 - u8.d * c2)) == c2 + (cc2 + u8.d * c2))
		{
		  retval = (-y);
		  goto ret;
		}
	    }
	  else
	    {
	      /* Second stage tan */
	      if ((y = c1 + (cc1 - u7.d * c1)) == c1 + (cc1 + u7.d * c1))
		{
		  retval = y;
		  goto ret;
		}
	    }
	  retval = tanMp (x);
	  goto ret;
	}

      /* (VII) The case 0.787 < abs(x) <= 25,    0.0608 < abs(y) <= 0.787 */

      /* First stage */
      i = ((int) (mfftnhf.d + TWO8 * ya));
      z = (z0 = (ya - xfg[i][0].d)) + yya;
      z2 = z * z;
      pz = z + z * z2 * (e0.d + z2 * e1.d);
      fi = xfg[i][1].d;
      gi = xfg[i][2].d;

      if (n)
	{
	  /* -cot */
	  t2 = pz * (fi + gi) / (fi + pz);
	  if ((y = gi - (t2 - gi * u10.d)) == gi - (t2 + gi * u10.d))
	    {
	      retval = (-sy * y);
	      goto ret;
	    }
	  t3 = (t2 < 0.0) ? -t2 : t2;
	  t4 = gi * ua10.d + t3 * ub10.d;
	  if ((y = gi - (t2 - t4)) == gi - (t2 + t4))
	    {
	      retval = (-sy * y);
	      goto ret;
	    }
	}
      else
	{
	  /* tan */
	  t2 = pz * (gi + fi) / (gi - pz);
	  if ((y = fi + (t2 - fi * u9.d)) == fi + (t2 + fi * u9.d))
	    {
	      retval = (sy * y);
	      goto ret;
	    }
	  t3 = (t2 < 0.0) ? -t2 : t2;
	  t4 = fi * ua9.d + t3 * ub9.d;
	  if ((y = fi + (t2 - t4)) == fi + (t2 + t4))
	    {
	      retval = (sy * y);
	      goto ret;
	    }
	}

      /* Second stage */
      ffi = xfg[i][3].d;
      EADD (z0, yya, z, zz)
      MUL2 (z, zz, z, zz, z2, zz2, t1, t2, t3, t4, t5, t6, t7, t8);
      c1 = z2 * (a7.d + z2 * (a9.d + z2 * a11.d));
      ADD2 (a5.d, aa5.d, c1, 0.0, c2, cc2, t1, t2);
      MUL2 (z2, zz2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (a3.d, aa3.d, c1, cc1, c2, cc2, t1, t2);
      MUL2 (z2, zz2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
      MUL2 (z, zz, c1, cc1, c2, cc2, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (z, zz, c2, cc2, c1, cc1, t1, t2);

      ADD2 (fi, ffi, c1, cc1, c2, cc2, t1, t2);
      MUL2 (fi, ffi, c1, cc1, c3, cc3, t1, t2, t3, t4, t5, t6, t7, t8);
      SUB2 (1.0, 0.0, c3, cc3, c1, cc1, t1, t2);

      if (n)
	{
	  /* -cot */
	  DIV2 (c1, cc1, c2, cc2, c3, cc3, t1, t2, t3, t4, t5, t6, t7, t8, t9,
		t10);
	  if ((y = c3 + (cc3 - u12.d * c3)) == c3 + (cc3 + u12.d * c3))
	    {
	      retval = (-sy * y);
	      goto ret;
	    }
	}
      else
	{
	  /* tan */
	  DIV2 (c2, cc2, c1, cc1, c3, cc3, t1, t2, t3, t4, t5, t6, t7, t8, t9,
		t10);
	  if ((y = c3 + (cc3 - u11.d * c3)) == c3 + (cc3 + u11.d * c3))
	    {
	      retval = (sy * y);
	      goto ret;
	    }
	}

      retval = tanMp (x);
      goto ret;
    }

  /* (---) The case 25 < abs(x) <= 1e8 */
  if (w <= g5.d)
    {
      /* Range reduction by algorithm ii */
      t = (x * hpinv.d + toint.d);
      xn = t - toint.d;
      v.d = t;
      t1 = (x - xn * mp1.d) - xn * mp2.d;
      n = v.i[LOW_HALF] & 0x00000001;
      da = xn * pp3.d;
      t = t1 - da;
      da = (t1 - t) - da;
      t1 = xn * pp4.d;
      a = t - t1;
      da = ((t - a) - t1) + da;
      EADD (a, da, t1, t2);
      a = t1;
      da = t2;
      if (a < 0.0)
	{
	  ya = -a;
	  yya = -da;
	  sy = -1;
	}
      else
	{
	  ya = a;
	  yya = da;
	  sy = 1;
	}

      /* (+++) The case 25 < abs(x) <= 1e8,    abs(y) <= 1e-7 */
      if (ya <= gy1.d)
	{
	  retval = tanMp (x);
	  goto ret;
	}

      /* (VIII) The case 25 < abs(x) <= 1e8,    1e-7 < abs(y) <= 0.0608 */
      if (ya <= gy2.d)
	{
	  a2 = a * a;
	  t2 = d9.d + a2 * d11.d;
	  t2 = d7.d + a2 * t2;
	  t2 = d5.d + a2 * t2;
	  t2 = d3.d + a2 * t2;
	  t2 = da + a * a2 * t2;

	  if (n)
	    {
	      /* First stage -cot */
	      EADD (a, t2, b, db);
	      DIV2 (1.0, 0.0, b, db, c, dc, t1, t2, t3, t4, t5, t6, t7, t8,
		    t9, t10);
	      if ((y = c + (dc - u14.d * c)) == c + (dc + u14.d * c))
		{
		  retval = (-y);
		  goto ret;
		}
	    }
	  else
	    {
	      /* First stage tan */
	      if ((y = a + (t2 - u13.d * a)) == a + (t2 + u13.d * a))
		{
		  retval = y;
		  goto ret;
		}
	    }

	  /* Second stage */
	  MUL2 (a, da, a, da, x2, xx2, t1, t2, t3, t4, t5, t6, t7, t8);
	  c1 = a25.d + x2 * a27.d;
	  c1 = a23.d + x2 * c1;
	  c1 = a21.d + x2 * c1;
	  c1 = a19.d + x2 * c1;
	  c1 = a17.d + x2 * c1;
	  c1 = a15.d + x2 * c1;
	  c1 *= x2;

	  ADD2 (a13.d, aa13.d, c1, 0.0, c2, cc2, t1, t2);
	  MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (a11.d, aa11.d, c1, cc1, c2, cc2, t1, t2);
	  MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (a9.d, aa9.d, c1, cc1, c2, cc2, t1, t2);
	  MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (a7.d, aa7.d, c1, cc1, c2, cc2, t1, t2);
	  MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (a5.d, aa5.d, c1, cc1, c2, cc2, t1, t2);
	  MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (a3.d, aa3.d, c1, cc1, c2, cc2, t1, t2);
	  MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
	  MUL2 (a, da, c1, cc1, c2, cc2, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (a, da, c2, cc2, c1, cc1, t1, t2);

	  if (n)
	    {
	      /* Second stage -cot */
	      DIV2 (1.0, 0.0, c1, cc1, c2, cc2, t1, t2, t3, t4, t5, t6, t7,
		    t8, t9, t10);
	      if ((y = c2 + (cc2 - u16.d * c2)) == c2 + (cc2 + u16.d * c2))
		{
		  retval = (-y);
		  goto ret;
		}
	    }
	  else
	    {
	      /* Second stage tan */
	      if ((y = c1 + (cc1 - u15.d * c1)) == c1 + (cc1 + u15.d * c1))
		{
		  retval = (y);
		  goto ret;
		}
	    }
	  retval = tanMp (x);
	  goto ret;
	}

      /* (IX) The case 25 < abs(x) <= 1e8,    0.0608 < abs(y) <= 0.787 */
      /* First stage */
      i = ((int) (mfftnhf.d + TWO8 * ya));
      z = (z0 = (ya - xfg[i][0].d)) + yya;
      z2 = z * z;
      pz = z + z * z2 * (e0.d + z2 * e1.d);
      fi = xfg[i][1].d;
      gi = xfg[i][2].d;

      if (n)
	{
	  /* -cot */
	  t2 = pz * (fi + gi) / (fi + pz);
	  if ((y = gi - (t2 - gi * u18.d)) == gi - (t2 + gi * u18.d))
	    {
	      retval = (-sy * y);
	      goto ret;
	    }
	  t3 = (t2 < 0.0) ? -t2 : t2;
	  t4 = gi * ua18.d + t3 * ub18.d;
	  if ((y = gi - (t2 - t4)) == gi - (t2 + t4))
	    {
	      retval = (-sy * y);
	      goto ret;
	    }
	}
      else
	{
	  /* tan */
	  t2 = pz * (gi + fi) / (gi - pz);
	  if ((y = fi + (t2 - fi * u17.d)) == fi + (t2 + fi * u17.d))
	    {
	      retval = (sy * y);
	      goto ret;
	    }
	  t3 = (t2 < 0.0) ? -t2 : t2;
	  t4 = fi * ua17.d + t3 * ub17.d;
	  if ((y = fi + (t2 - t4)) == fi + (t2 + t4))
	    {
	      retval = (sy * y);
	      goto ret;
	    }
	}

      /* Second stage */
      ffi = xfg[i][3].d;
      EADD (z0, yya, z, zz);
      MUL2 (z, zz, z, zz, z2, zz2, t1, t2, t3, t4, t5, t6, t7, t8);
      c1 = z2 * (a7.d + z2 * (a9.d + z2 * a11.d));
      ADD2 (a5.d, aa5.d, c1, 0.0, c2, cc2, t1, t2);
      MUL2 (z2, zz2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (a3.d, aa3.d, c1, cc1, c2, cc2, t1, t2);
      MUL2 (z2, zz2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
      MUL2 (z, zz, c1, cc1, c2, cc2, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (z, zz, c2, cc2, c1, cc1, t1, t2);

      ADD2 (fi, ffi, c1, cc1, c2, cc2, t1, t2);
      MUL2 (fi, ffi, c1, cc1, c3, cc3, t1, t2, t3, t4, t5, t6, t7, t8);
      SUB2 (1.0, 0.0, c3, cc3, c1, cc1, t1, t2);

      if (n)
	{
	  /* -cot */
	  DIV2 (c1, cc1, c2, cc2, c3, cc3, t1, t2, t3, t4, t5, t6, t7, t8, t9,
		t10);
	  if ((y = c3 + (cc3 - u20.d * c3)) == c3 + (cc3 + u20.d * c3))
	    {
	      retval = (-sy * y);
	      goto ret;
	    }
	}
      else
	{
	  /* tan */
	  DIV2 (c2, cc2, c1, cc1, c3, cc3, t1, t2, t3, t4, t5, t6, t7, t8, t9,
		t10);
	  if ((y = c3 + (cc3 - u19.d * c3)) == c3 + (cc3 + u19.d * c3))
	    {
	      retval = (sy * y);
	      goto ret;
	    }
	}
      retval = tanMp (x);
      goto ret;
    }

  /* (---) The case 1e8 < abs(x) < 2**1024 */
  /* Range reduction by algorithm iii */
  n = (__branred (x, &a, &da)) & 0x00000001;
  EADD (a, da, t1, t2);
  a = t1;
  da = t2;
  if (a < 0.0)
    {
      ya = -a;
      yya = -da;
      sy = -1;
    }
  else
    {
      ya = a;
      yya = da;
      sy = 1;
    }

  /* (+++) The case 1e8 < abs(x) < 2**1024,    abs(y) <= 1e-7 */
  if (ya <= gy1.d)
    {
      retval = tanMp (x);
      goto ret;
    }

  /* (X) The case 1e8 < abs(x) < 2**1024,    1e-7 < abs(y) <= 0.0608 */
  if (ya <= gy2.d)
    {
      a2 = a * a;
      t2 = d9.d + a2 * d11.d;
      t2 = d7.d + a2 * t2;
      t2 = d5.d + a2 * t2;
      t2 = d3.d + a2 * t2;
      t2 = da + a * a2 * t2;
      if (n)
	{
	  /* First stage -cot */
	  EADD (a, t2, b, db);
	  DIV2 (1.0, 0.0, b, db, c, dc, t1, t2, t3, t4, t5, t6, t7, t8, t9,
		t10);
	  if ((y = c + (dc - u22.d * c)) == c + (dc + u22.d * c))
	    {
	      retval = (-y);
	      goto ret;
	    }
	}
      else
	{
	  /* First stage tan */
	  if ((y = a + (t2 - u21.d * a)) == a + (t2 + u21.d * a))
	    {
	      retval = y;
	      goto ret;
	    }
	}

      /* Second stage */
      /* Reduction by algorithm iv */
      p = 10;
      n = (__mpranred (x, &mpa, p)) & 0x00000001;
      __mp_dbl (&mpa, &a, p);
      __dbl_mp (a, &mpt1, p);
      __sub (&mpa, &mpt1, &mpt2, p);
      __mp_dbl (&mpt2, &da, p);

      MUL2 (a, da, a, da, x2, xx2, t1, t2, t3, t4, t5, t6, t7, t8);

      c1 = a25.d + x2 * a27.d;
      c1 = a23.d + x2 * c1;
      c1 = a21.d + x2 * c1;
      c1 = a19.d + x2 * c1;
      c1 = a17.d + x2 * c1;
      c1 = a15.d + x2 * c1;
      c1 *= x2;

      ADD2 (a13.d, aa13.d, c1, 0.0, c2, cc2, t1, t2);
      MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (a11.d, aa11.d, c1, cc1, c2, cc2, t1, t2);
      MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (a9.d, aa9.d, c1, cc1, c2, cc2, t1, t2);
      MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (a7.d, aa7.d, c1, cc1, c2, cc2, t1, t2);
      MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (a5.d, aa5.d, c1, cc1, c2, cc2, t1, t2);
      MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (a3.d, aa3.d, c1, cc1, c2, cc2, t1, t2);
      MUL2 (x2, xx2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
      MUL2 (a, da, c1, cc1, c2, cc2, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (a, da, c2, cc2, c1, cc1, t1, t2);

      if (n)
	{
	  /* Second stage -cot */
	  DIV2 (1.0, 0.0, c1, cc1, c2, cc2, t1, t2, t3, t4, t5, t6, t7, t8,
		t9, t10);
	  if ((y = c2 + (cc2 - u24.d * c2)) == c2 + (cc2 + u24.d * c2))
	    {
	      retval = (-y);
	      goto ret;
	    }
	}
      else
	{
	  /* Second stage tan */
	  if ((y = c1 + (cc1 - u23.d * c1)) == c1 + (cc1 + u23.d * c1))
	    {
	      retval = y;
	      goto ret;
	    }
	}
      retval = tanMp (x);
      goto ret;
    }

  /* (XI) The case 1e8 < abs(x) < 2**1024,    0.0608 < abs(y) <= 0.787 */
  /* First stage */
  i = ((int) (mfftnhf.d + TWO8 * ya));
  z = (z0 = (ya - xfg[i][0].d)) + yya;
  z2 = z * z;
  pz = z + z * z2 * (e0.d + z2 * e1.d);
  fi = xfg[i][1].d;
  gi = xfg[i][2].d;

  if (n)
    {
      /* -cot */
      t2 = pz * (fi + gi) / (fi + pz);
      if ((y = gi - (t2 - gi * u26.d)) == gi - (t2 + gi * u26.d))
	{
	  retval = (-sy * y);
	  goto ret;
	}
      t3 = (t2 < 0.0) ? -t2 : t2;
      t4 = gi * ua26.d + t3 * ub26.d;
      if ((y = gi - (t2 - t4)) == gi - (t2 + t4))
	{
	  retval = (-sy * y);
	  goto ret;
	}
    }
  else
    {
      /* tan */
      t2 = pz * (gi + fi) / (gi - pz);
      if ((y = fi + (t2 - fi * u25.d)) == fi + (t2 + fi * u25.d))
	{
	  retval = (sy * y);
	  goto ret;
	}
      t3 = (t2 < 0.0) ? -t2 : t2;
      t4 = fi * ua25.d + t3 * ub25.d;
      if ((y = fi + (t2 - t4)) == fi + (t2 + t4))
	{
	  retval = (sy * y);
	  goto ret;
	}
    }

  /* Second stage */
  ffi = xfg[i][3].d;
  EADD (z0, yya, z, zz);
  MUL2 (z, zz, z, zz, z2, zz2, t1, t2, t3, t4, t5, t6, t7, t8);
  c1 = z2 * (a7.d + z2 * (a9.d + z2 * a11.d));
  ADD2 (a5.d, aa5.d, c1, 0.0, c2, cc2, t1, t2);
  MUL2 (z2, zz2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
  ADD2 (a3.d, aa3.d, c1, cc1, c2, cc2, t1, t2);
  MUL2 (z2, zz2, c2, cc2, c1, cc1, t1, t2, t3, t4, t5, t6, t7, t8);
  MUL2 (z, zz, c1, cc1, c2, cc2, t1, t2, t3, t4, t5, t6, t7, t8);
  ADD2 (z, zz, c2, cc2, c1, cc1, t1, t2);

  ADD2 (fi, ffi, c1, cc1, c2, cc2, t1, t2);
  MUL2 (fi, ffi, c1, cc1, c3, cc3, t1, t2, t3, t4, t5, t6, t7, t8);
  SUB2 (1.0, 0.0, c3, cc3, c1, cc1, t1, t2);

  if (n)
    {
      /* -cot */
      DIV2 (c1, cc1, c2, cc2, c3, cc3, t1, t2, t3, t4, t5, t6, t7, t8, t9,
	    t10);
      if ((y = c3 + (cc3 - u28.d * c3)) == c3 + (cc3 + u28.d * c3))
	{
	  retval = (-sy * y);
	  goto ret;
	}
    }
  else
    {
      /* tan */
      DIV2 (c2, cc2, c1, cc1, c3, cc3, t1, t2, t3, t4, t5, t6, t7, t8, t9,
	    t10);
      if ((y = c3 + (cc3 - u27.d * c3)) == c3 + (cc3 + u27.d * c3))
	{
	  retval = (sy * y);
	  goto ret;
	}
    }
  retval = tanMp (x);
  goto ret;

ret:
  return retval;
}

/* multiple precision stage                                              */
/* Convert x to multi precision number,compute tan(x) by mptan() routine */
/* and converts result back to double                                    */
static double
SECTION
tanMp (double x)
{
  int p;
  double y;
  mp_no mpy;
  p = 32;
  __mptan (x, &mpy, p);
  __mp_dbl (&mpy, &y, p);
  LIBC_PROBE (slowtan, 2, &x, &y);
  return y;
}

#ifndef __tan
libm_alias_double (__tan, tan)
#endif
