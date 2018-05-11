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
/*  MODULE_NAME: atnat2.c                                               */
/*                                                                      */
/*  FUNCTIONS: uatan2                                                   */
/*             atan2Mp                                                  */
/*             signArctan2                                              */
/*             normalized                                               */
/*                                                                      */
/*  FILES NEEDED: dla.h endian.h mpa.h mydefs.h atnat2.h                */
/*                mpatan.c mpatan2.c mpsqrt.c                           */
/*                uatan.tbl                                             */
/*                                                                      */
/* An ultimate atan2() routine. Given two IEEE double machine numbers y,*/
/* x it computes the correctly rounded (to nearest) value of atan2(y,x).*/
/*                                                                      */
/* Assumption: Machine arithmetic operations are performed in           */
/* round to nearest mode of IEEE 754 standard.                          */
/*                                                                      */
/************************************************************************/

#include <dla.h>
#include "mpa.h"
#include "MathLib.h"
#include "uatan.tbl"
#include "atnat2.h"
#include <fenv.h>
#include <float.h>
#include <math.h>
#include <math-barriers.h>
#include <math_private.h>
#include <stap-probe.h>

#ifndef SECTION
# define SECTION
#endif

/************************************************************************/
/* An ultimate atan2 routine. Given two IEEE double machine numbers y,x */
/* it computes the correctly rounded (to nearest) value of atan2(y,x).  */
/* Assumption: Machine arithmetic operations are performed in           */
/* round to nearest mode of IEEE 754 standard.                          */
/************************************************************************/
static double atan2Mp (double, double, const int[]);
  /* Fix the sign and return after stage 1 or stage 2 */
static double
signArctan2 (double y, double z)
{
  return __copysign (z, y);
}

static double normalized (double, double, double, double);
void __mpatan2 (mp_no *, mp_no *, mp_no *, int);

double
SECTION
__ieee754_atan2 (double y, double x)
{
  int i, de, ux, dx, uy, dy;
  static const int pr[MM] = { 6, 8, 10, 20, 32 };
  double ax, ay, u, du, u9, ua, v, vv, dv, t1, t2, t3, t7, t8,
	 z, zz, cor, s1, ss1, s2, ss2;
#ifndef DLA_FMS
  double t4, t5, t6;
#endif
  number num;

  static const int ep = 59768832,      /*  57*16**5   */
		   em = -59768832;      /* -57*16**5   */

  /* x=NaN or y=NaN */
  num.d = x;
  ux = num.i[HIGH_HALF];
  dx = num.i[LOW_HALF];
  if ((ux & 0x7ff00000) == 0x7ff00000)
    {
      if (((ux & 0x000fffff) | dx) != 0x00000000)
	return x + y;
    }
  num.d = y;
  uy = num.i[HIGH_HALF];
  dy = num.i[LOW_HALF];
  if ((uy & 0x7ff00000) == 0x7ff00000)
    {
      if (((uy & 0x000fffff) | dy) != 0x00000000)
	return y + y;
    }

  /* y=+-0 */
  if (uy == 0x00000000)
    {
      if (dy == 0x00000000)
	{
	  if ((ux & 0x80000000) == 0x00000000)
	    return 0;
	  else
	    return opi.d;
	}
    }
  else if (uy == 0x80000000)
    {
      if (dy == 0x00000000)
	{
	  if ((ux & 0x80000000) == 0x00000000)
	    return -0.0;
	  else
	    return mopi.d;
	}
    }

  /* x=+-0 */
  if (x == 0)
    {
      if ((uy & 0x80000000) == 0x00000000)
	return hpi.d;
      else
	return mhpi.d;
    }

  /* x=+-INF */
  if (ux == 0x7ff00000)
    {
      if (dx == 0x00000000)
	{
	  if (uy == 0x7ff00000)
	    {
	      if (dy == 0x00000000)
		return qpi.d;
	    }
	  else if (uy == 0xfff00000)
	    {
	      if (dy == 0x00000000)
		return mqpi.d;
	    }
	  else
	    {
	      if ((uy & 0x80000000) == 0x00000000)
		return 0;
	      else
		return -0.0;
	    }
	}
    }
  else if (ux == 0xfff00000)
    {
      if (dx == 0x00000000)
	{
	  if (uy == 0x7ff00000)
	    {
	      if (dy == 0x00000000)
		return tqpi.d;
	    }
	  else if (uy == 0xfff00000)
	    {
	      if (dy == 0x00000000)
		return mtqpi.d;
	    }
	  else
	    {
	      if ((uy & 0x80000000) == 0x00000000)
		return opi.d;
	      else
		return mopi.d;
	    }
	}
    }

  /* y=+-INF */
  if (uy == 0x7ff00000)
    {
      if (dy == 0x00000000)
	return hpi.d;
    }
  else if (uy == 0xfff00000)
    {
      if (dy == 0x00000000)
	return mhpi.d;
    }

  SET_RESTORE_ROUND (FE_TONEAREST);
  /* either x/y or y/x is very close to zero */
  ax = (x < 0) ? -x : x;
  ay = (y < 0) ? -y : y;
  de = (uy & 0x7ff00000) - (ux & 0x7ff00000);
  if (de >= ep)
    {
      return ((y > 0) ? hpi.d : mhpi.d);
    }
  else if (de <= em)
    {
      if (x > 0)
	{
	  double ret;
	  if ((z = ay / ax) < TWOM1022)
	    ret = normalized (ax, ay, y, z);
	  else
	    ret = signArctan2 (y, z);
	  if (fabs (ret) < DBL_MIN)
	    {
	      double vret = ret ? ret : DBL_MIN;
	      double force_underflow = vret * vret;
	      math_force_eval (force_underflow);
	    }
	  return ret;
	}
      else
	{
	  return ((y > 0) ? opi.d : mopi.d);
	}
    }

  /* if either x or y is extremely close to zero, scale abs(x), abs(y). */
  if (ax < twom500.d || ay < twom500.d)
    {
      ax *= two500.d;
      ay *= two500.d;
    }

  /* Likewise for large x and y.  */
  if (ax > two500.d || ay > two500.d)
    {
      ax *= twom500.d;
      ay *= twom500.d;
    }

  /* x,y which are neither special nor extreme */
  if (ay < ax)
    {
      u = ay / ax;
      EMULV (ax, u, v, vv, t1, t2, t3, t4, t5);
      du = ((ay - v) - vv) / ax;
    }
  else
    {
      u = ax / ay;
      EMULV (ay, u, v, vv, t1, t2, t3, t4, t5);
      du = ((ax - v) - vv) / ay;
    }

  if (x > 0)
    {
      /* (i)   x>0, abs(y)< abs(x):  atan(ay/ax) */
      if (ay < ax)
	{
	  if (u < inv16.d)
	    {
	      v = u * u;

	      zz = du + u * v * (d3.d
				 + v * (d5.d
					+ v * (d7.d
					       + v * (d9.d
						      + v * (d11.d
							     + v * d13.d)))));

	      if ((z = u + (zz - u1.d * u)) == u + (zz + u1.d * u))
		return signArctan2 (y, z);

	      MUL2 (u, du, u, du, v, vv, t1, t2, t3, t4, t5, t6, t7, t8);
	      s1 = v * (f11.d + v * (f13.d
				     + v * (f15.d + v * (f17.d + v * f19.d))));
	      ADD2 (f9.d, ff9.d, s1, 0, s2, ss2, t1, t2);
	      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	      ADD2 (f7.d, ff7.d, s1, ss1, s2, ss2, t1, t2);
	      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	      ADD2 (f5.d, ff5.d, s1, ss1, s2, ss2, t1, t2);
	      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	      ADD2 (f3.d, ff3.d, s1, ss1, s2, ss2, t1, t2);
	      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	      MUL2 (u, du, s1, ss1, s2, ss2, t1, t2, t3, t4, t5, t6, t7, t8);
	      ADD2 (u, du, s2, ss2, s1, ss1, t1, t2);

	      if ((z = s1 + (ss1 - u5.d * s1)) == s1 + (ss1 + u5.d * s1))
		return signArctan2 (y, z);

	      return atan2Mp (x, y, pr);
	    }

	  i = (TWO52 + TWO8 * u) - TWO52;
	  i -= 16;
	  t3 = u - cij[i][0].d;
	  EADD (t3, du, v, dv);
	  t1 = cij[i][1].d;
	  t2 = cij[i][2].d;
	  zz = v * t2 + (dv * t2
			 + v * v * (cij[i][3].d
				    + v * (cij[i][4].d
					   + v * (cij[i][5].d
						  + v * cij[i][6].d))));
	  if (i < 112)
	    {
	      if (i < 48)
		u9 = u91.d;     /* u < 1/4	*/
	      else
		u9 = u92.d;
	    }           /* 1/4 <= u < 1/2 */
	  else
	    {
	      if (i < 176)
		u9 = u93.d;     /* 1/2 <= u < 3/4 */
	      else
		u9 = u94.d;
	    }           /* 3/4 <= u <= 1  */
	  if ((z = t1 + (zz - u9 * t1)) == t1 + (zz + u9 * t1))
	    return signArctan2 (y, z);

	  t1 = u - hij[i][0].d;
	  EADD (t1, du, v, vv);
	  s1 = v * (hij[i][11].d
		    + v * (hij[i][12].d
			   + v * (hij[i][13].d
				  + v * (hij[i][14].d
					 + v * hij[i][15].d))));
	  ADD2 (hij[i][9].d, hij[i][10].d, s1, 0, s2, ss2, t1, t2);
	  MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (hij[i][7].d, hij[i][8].d, s1, ss1, s2, ss2, t1, t2);
	  MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (hij[i][5].d, hij[i][6].d, s1, ss1, s2, ss2, t1, t2);
	  MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (hij[i][3].d, hij[i][4].d, s1, ss1, s2, ss2, t1, t2);
	  MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (hij[i][1].d, hij[i][2].d, s1, ss1, s2, ss2, t1, t2);

	  if ((z = s2 + (ss2 - ub.d * s2)) == s2 + (ss2 + ub.d * s2))
	    return signArctan2 (y, z);
	  return atan2Mp (x, y, pr);
	}

      /* (ii)  x>0, abs(x)<=abs(y):  pi/2-atan(ax/ay) */
      if (u < inv16.d)
	{
	  v = u * u;
	  zz = u * v * (d3.d
			+ v * (d5.d
			       + v * (d7.d
				      + v * (d9.d
					     + v * (d11.d
						    + v * d13.d)))));
	  ESUB (hpi.d, u, t2, cor);
	  t3 = ((hpi1.d + cor) - du) - zz;
	  if ((z = t2 + (t3 - u2.d)) == t2 + (t3 + u2.d))
	    return signArctan2 (y, z);

	  MUL2 (u, du, u, du, v, vv, t1, t2, t3, t4, t5, t6, t7, t8);
	  s1 = v * (f11.d
		    + v * (f13.d
			   + v * (f15.d + v * (f17.d + v * f19.d))));
	  ADD2 (f9.d, ff9.d, s1, 0, s2, ss2, t1, t2);
	  MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (f7.d, ff7.d, s1, ss1, s2, ss2, t1, t2);
	  MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (f5.d, ff5.d, s1, ss1, s2, ss2, t1, t2);
	  MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (f3.d, ff3.d, s1, ss1, s2, ss2, t1, t2);
	  MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  MUL2 (u, du, s1, ss1, s2, ss2, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (u, du, s2, ss2, s1, ss1, t1, t2);
	  SUB2 (hpi.d, hpi1.d, s1, ss1, s2, ss2, t1, t2);

	  if ((z = s2 + (ss2 - u6.d)) == s2 + (ss2 + u6.d))
	    return signArctan2 (y, z);
	  return atan2Mp (x, y, pr);
	}

      i = (TWO52 + TWO8 * u) - TWO52;
      i -= 16;
      v = (u - cij[i][0].d) + du;

      zz = hpi1.d - v * (cij[i][2].d
			 + v * (cij[i][3].d
				+ v * (cij[i][4].d
				       + v * (cij[i][5].d
					      + v * cij[i][6].d))));
      t1 = hpi.d - cij[i][1].d;
      if (i < 112)
	ua = ua1.d;	/* w <  1/2 */
      else
	ua = ua2.d;	/* w >= 1/2 */
      if ((z = t1 + (zz - ua)) == t1 + (zz + ua))
	return signArctan2 (y, z);

      t1 = u - hij[i][0].d;
      EADD (t1, du, v, vv);

      s1 = v * (hij[i][11].d
		+ v * (hij[i][12].d
		       + v * (hij[i][13].d
			      + v * (hij[i][14].d
				     + v * hij[i][15].d))));

      ADD2 (hij[i][9].d, hij[i][10].d, s1, 0, s2, ss2, t1, t2);
      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (hij[i][7].d, hij[i][8].d, s1, ss1, s2, ss2, t1, t2);
      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (hij[i][5].d, hij[i][6].d, s1, ss1, s2, ss2, t1, t2);
      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (hij[i][3].d, hij[i][4].d, s1, ss1, s2, ss2, t1, t2);
      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (hij[i][1].d, hij[i][2].d, s1, ss1, s2, ss2, t1, t2);
      SUB2 (hpi.d, hpi1.d, s2, ss2, s1, ss1, t1, t2);

      if ((z = s1 + (ss1 - uc.d)) == s1 + (ss1 + uc.d))
	return signArctan2 (y, z);
      return atan2Mp (x, y, pr);
    }

  /* (iii) x<0, abs(x)< abs(y):  pi/2+atan(ax/ay) */
  if (ax < ay)
    {
      if (u < inv16.d)
	{
	  v = u * u;
	  zz = u * v * (d3.d
			+ v * (d5.d
			       + v * (d7.d
				      + v * (d9.d
					     + v * (d11.d + v * d13.d)))));
	  EADD (hpi.d, u, t2, cor);
	  t3 = ((hpi1.d + cor) + du) + zz;
	  if ((z = t2 + (t3 - u3.d)) == t2 + (t3 + u3.d))
	    return signArctan2 (y, z);

	  MUL2 (u, du, u, du, v, vv, t1, t2, t3, t4, t5, t6, t7, t8);
	  s1 = v * (f11.d
		    + v * (f13.d + v * (f15.d + v * (f17.d + v * f19.d))));
	  ADD2 (f9.d, ff9.d, s1, 0, s2, ss2, t1, t2);
	  MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (f7.d, ff7.d, s1, ss1, s2, ss2, t1, t2);
	  MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (f5.d, ff5.d, s1, ss1, s2, ss2, t1, t2);
	  MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (f3.d, ff3.d, s1, ss1, s2, ss2, t1, t2);
	  MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
	  MUL2 (u, du, s1, ss1, s2, ss2, t1, t2, t3, t4, t5, t6, t7, t8);
	  ADD2 (u, du, s2, ss2, s1, ss1, t1, t2);
	  ADD2 (hpi.d, hpi1.d, s1, ss1, s2, ss2, t1, t2);

	  if ((z = s2 + (ss2 - u7.d)) == s2 + (ss2 + u7.d))
	    return signArctan2 (y, z);
	  return atan2Mp (x, y, pr);
	}

      i = (TWO52 + TWO8 * u) - TWO52;
      i -= 16;
      v = (u - cij[i][0].d) + du;
      zz = hpi1.d + v * (cij[i][2].d
			 + v * (cij[i][3].d
				+ v * (cij[i][4].d
				       + v * (cij[i][5].d
					      + v * cij[i][6].d))));
      t1 = hpi.d + cij[i][1].d;
      if (i < 112)
	ua = ua1.d;	/* w <  1/2 */
      else
	ua = ua2.d;	/* w >= 1/2 */
      if ((z = t1 + (zz - ua)) == t1 + (zz + ua))
	return signArctan2 (y, z);

      t1 = u - hij[i][0].d;
      EADD (t1, du, v, vv);
      s1 = v * (hij[i][11].d
		+ v * (hij[i][12].d
		       + v * (hij[i][13].d
			      + v * (hij[i][14].d
				     + v * hij[i][15].d))));
      ADD2 (hij[i][9].d, hij[i][10].d, s1, 0, s2, ss2, t1, t2);
      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (hij[i][7].d, hij[i][8].d, s1, ss1, s2, ss2, t1, t2);
      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (hij[i][5].d, hij[i][6].d, s1, ss1, s2, ss2, t1, t2);
      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (hij[i][3].d, hij[i][4].d, s1, ss1, s2, ss2, t1, t2);
      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (hij[i][1].d, hij[i][2].d, s1, ss1, s2, ss2, t1, t2);
      ADD2 (hpi.d, hpi1.d, s2, ss2, s1, ss1, t1, t2);

      if ((z = s1 + (ss1 - uc.d)) == s1 + (ss1 + uc.d))
	return signArctan2 (y, z);
      return atan2Mp (x, y, pr);
    }

  /* (iv)  x<0, abs(y)<=abs(x):  pi-atan(ax/ay) */
  if (u < inv16.d)
    {
      v = u * u;
      zz = u * v * (d3.d
		    + v * (d5.d
			   + v * (d7.d
				  + v * (d9.d + v * (d11.d + v * d13.d)))));
      ESUB (opi.d, u, t2, cor);
      t3 = ((opi1.d + cor) - du) - zz;
      if ((z = t2 + (t3 - u4.d)) == t2 + (t3 + u4.d))
	return signArctan2 (y, z);

      MUL2 (u, du, u, du, v, vv, t1, t2, t3, t4, t5, t6, t7, t8);
      s1 = v * (f11.d + v * (f13.d + v * (f15.d + v * (f17.d + v * f19.d))));
      ADD2 (f9.d, ff9.d, s1, 0, s2, ss2, t1, t2);
      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (f7.d, ff7.d, s1, ss1, s2, ss2, t1, t2);
      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (f5.d, ff5.d, s1, ss1, s2, ss2, t1, t2);
      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (f3.d, ff3.d, s1, ss1, s2, ss2, t1, t2);
      MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
      MUL2 (u, du, s1, ss1, s2, ss2, t1, t2, t3, t4, t5, t6, t7, t8);
      ADD2 (u, du, s2, ss2, s1, ss1, t1, t2);
      SUB2 (opi.d, opi1.d, s1, ss1, s2, ss2, t1, t2);

      if ((z = s2 + (ss2 - u8.d)) == s2 + (ss2 + u8.d))
	return signArctan2 (y, z);
      return atan2Mp (x, y, pr);
    }

  i = (TWO52 + TWO8 * u) - TWO52;
  i -= 16;
  v = (u - cij[i][0].d) + du;
  zz = opi1.d - v * (cij[i][2].d
		     + v * (cij[i][3].d
			    + v * (cij[i][4].d
				   + v * (cij[i][5].d + v * cij[i][6].d))));
  t1 = opi.d - cij[i][1].d;
  if (i < 112)
    ua = ua1.d;	/* w <  1/2 */
  else
    ua = ua2.d;	/* w >= 1/2 */
  if ((z = t1 + (zz - ua)) == t1 + (zz + ua))
    return signArctan2 (y, z);

  t1 = u - hij[i][0].d;

  EADD (t1, du, v, vv);

  s1 = v * (hij[i][11].d
	    + v * (hij[i][12].d
		   + v * (hij[i][13].d
			  + v * (hij[i][14].d + v * hij[i][15].d))));

  ADD2 (hij[i][9].d, hij[i][10].d, s1, 0, s2, ss2, t1, t2);
  MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
  ADD2 (hij[i][7].d, hij[i][8].d, s1, ss1, s2, ss2, t1, t2);
  MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
  ADD2 (hij[i][5].d, hij[i][6].d, s1, ss1, s2, ss2, t1, t2);
  MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
  ADD2 (hij[i][3].d, hij[i][4].d, s1, ss1, s2, ss2, t1, t2);
  MUL2 (v, vv, s2, ss2, s1, ss1, t1, t2, t3, t4, t5, t6, t7, t8);
  ADD2 (hij[i][1].d, hij[i][2].d, s1, ss1, s2, ss2, t1, t2);
  SUB2 (opi.d, opi1.d, s2, ss2, s1, ss1, t1, t2);

  if ((z = s1 + (ss1 - uc.d)) == s1 + (ss1 + uc.d))
    return signArctan2 (y, z);
  return atan2Mp (x, y, pr);
}

#ifndef __ieee754_atan2
strong_alias (__ieee754_atan2, __atan2_finite)
#endif

/* Treat the Denormalized case */
static double
SECTION
normalized (double ax, double ay, double y, double z)
{
  int p;
  mp_no mpx, mpy, mpz, mperr, mpz2, mpt1;
  p = 6;
  __dbl_mp (ax, &mpx, p);
  __dbl_mp (ay, &mpy, p);
  __dvd (&mpy, &mpx, &mpz, p);
  __dbl_mp (ue.d, &mpt1, p);
  __mul (&mpz, &mpt1, &mperr, p);
  __sub (&mpz, &mperr, &mpz2, p);
  __mp_dbl (&mpz2, &z, p);
  return signArctan2 (y, z);
}

/* Stage 3: Perform a multi-Precision computation */
static double
SECTION
atan2Mp (double x, double y, const int pr[])
{
  double z1, z2;
  int i, p;
  mp_no mpx, mpy, mpz, mpz1, mpz2, mperr, mpt1;
  for (i = 0; i < MM; i++)
    {
      p = pr[i];
      __dbl_mp (x, &mpx, p);
      __dbl_mp (y, &mpy, p);
      __mpatan2 (&mpy, &mpx, &mpz, p);
      __dbl_mp (ud[i].d, &mpt1, p);
      __mul (&mpz, &mpt1, &mperr, p);
      __add (&mpz, &mperr, &mpz1, p);
      __sub (&mpz, &mperr, &mpz2, p);
      __mp_dbl (&mpz1, &z1, p);
      __mp_dbl (&mpz2, &z2, p);
      if (z1 == z2)
	{
	  LIBC_PROBE (slowatan2, 4, &p, &x, &y, &z1);
	  return z1;
	}
    }
  LIBC_PROBE (slowatan2_inexact, 4, &p, &x, &y, &z1);
  return z1;			/*if impossible to do exact computing */
}
