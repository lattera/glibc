/*
 * IBM Accurate Mathematical Library
 * written by International Business Machines Corp.
 * Copyright (C) 2001, 2004, 2006, 2011 Free Software Foundation
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/*********************************************************************/
/* MODULE_NAME: uroot.c                                              */
/*                                                                   */
/* FUNCTION:    usqrt                                                */
/*                                                                   */
/* FILES NEEDED: dla.h endian.h mydefs.h uroot.h                     */
/*               uroot.tbl                                           */
/*                                                                   */
/* An ultimate sqrt routine. Given an IEEE double machine number x   */
/* it computes the correctly rounded (to nearest) value of square    */
/* root of x.                                                        */
/* Assumption: Machine arithmetic operations are performed in        */
/* round to nearest mode of IEEE 754 standard.                       */
/*                                                                   */
/*********************************************************************/

#include <math_private.h>

typedef unsigned int int4;
typedef union {int4 i[4]; long double x; double d[2]; } mynumber;

static const  mynumber
  t512 = {{0x5ff00000, 0x00000000, 0x00000000, 0x00000000 }},  /* 2^512  */
  tm256 = {{0x2ff00000, 0x00000000, 0x00000000, 0x00000000 }};  /* 2^-256 */
static const double
two54 = 1.80143985094819840000e+16, /* 0x4350000000000000 */
twom54 = 5.55111512312578270212e-17; /* 0x3C90000000000000 */

/*********************************************************************/
/* An ultimate sqrt routine. Given an IEEE double machine number x   */
/* it computes the correctly rounded (to nearest) value of square    */
/* root of x.                                                        */
/*********************************************************************/
long double __ieee754_sqrtl(long double x)
{
  static const long double big = 134217728.0, big1 = 134217729.0;
  long double t,s,i;
  mynumber a,c;
  int4 k, l, m;
  int n;
  double d;

  a.x=x;
  k=a.i[0] & 0x7fffffff;
  /*----------------- 2^-1022  <= | x |< 2^1024  -----------------*/
  if (k>0x000fffff && k<0x7ff00000) {
    if (x < 0) return (big1-big1)/(big-big);
    l = (k&0x001fffff)|0x3fe00000;
    if (((a.i[2] & 0x7fffffff) | a.i[3]) != 0) {
      n = (int) ((l - k) * 2) >> 21;
      m = (a.i[2] >> 20) & 0x7ff;
      if (m == 0) {
	a.d[1] *= two54;
	m = ((a.i[2] >> 20) & 0x7ff) - 54;
      }
      m += n;
      if ((int) m > 0)
	a.i[2] = (a.i[2] & 0x800fffff) | (m << 20);
      else if ((int) m <= -54) {
	a.i[2] &= 0x80000000;
	a.i[3] = 0;
      } else {
	m += 54;
	a.i[2] = (a.i[2] & 0x800fffff) | (m << 20);
	a.d[1] *= twom54;
      }
    }
    a.i[0] = l;
    s = a.x;
    d = __ieee754_sqrt (a.d[0]);
    c.i[0] = 0x20000000+((k&0x7fe00000)>>1);
    c.i[1] = 0;
    c.i[2] = 0;
    c.i[3] = 0;
    i = d;
    t = 0.5L * (i + s / i);
    i = 0.5L * (t + s / t);
    return c.x * i;
  }
  else {
    if (k>=0x7ff00000) {
      if (a.i[0] == 0xfff00000 && a.i[1] == 0)
	return (big1-big1)/(big-big); /* sqrt (-Inf) = NaN.  */
      return x; /* sqrt (NaN) = NaN, sqrt (+Inf) = +Inf.  */
    }
    if (x == 0) return x;
    if (x < 0) return (big1-big1)/(big-big);
    return tm256.x*__ieee754_sqrtl(x*t512.x);
  }
}
strong_alias (__ieee754_sqrtl, __sqrtl_finite)
