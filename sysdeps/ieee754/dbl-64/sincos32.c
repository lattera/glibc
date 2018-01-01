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
/****************************************************************/
/*  MODULE_NAME: sincos32.c                                     */
/*                                                              */
/*  FUNCTIONS: ss32                                             */
/*             cc32                                             */
/*             c32                                              */
/*             sin32                                            */
/*             cos32                                            */
/*             mpsin                                            */
/*             mpcos                                            */
/*             mpranred                                         */
/*             mpsin1                                           */
/*             mpcos1                                           */
/*                                                              */
/* FILES NEEDED: endian.h mpa.h sincos32.h                      */
/*               mpa.c                                          */
/*                                                              */
/* Multi Precision sin() and cos() function with p=32  for sin()*/
/* cos() arcsin() and arccos() routines                         */
/* In addition mpranred() routine  performs range  reduction of */
/* a double number x into multi precision number   y,           */
/* such that y=x-n*pi/2, abs(y)<pi/4,  n=0,+-1,+-2,....         */
/****************************************************************/
#include "endian.h"
#include "mpa.h"
#include "sincos32.h"
#include <math.h>
#include <math_private.h>
#include <stap-probe.h>

#ifndef SECTION
# define SECTION
#endif

/* Compute Multi-Precision sin() function for given p.  Receive Multi Precision
   number x and result stored at y.  */
static void
SECTION
ss32 (mp_no *x, mp_no *y, int p)
{
  int i;
  double a;
  mp_no mpt1, x2, gor, sum, mpk = {1, {1.0}};
  for (i = 1; i <= p; i++)
    mpk.d[i] = 0;

  __sqr (x, &x2, p);
  __cpy (&oofac27, &gor, p);
  __cpy (&gor, &sum, p);
  for (a = 27.0; a > 1.0; a -= 2.0)
    {
      mpk.d[1] = a * (a - 1.0);
      __mul (&gor, &mpk, &mpt1, p);
      __cpy (&mpt1, &gor, p);
      __mul (&x2, &sum, &mpt1, p);
      __sub (&gor, &mpt1, &sum, p);
    }
  __mul (x, &sum, y, p);
}

/* Compute Multi-Precision cos() function for given p. Receive Multi Precision
   number x and result stored at y.  */
static void
SECTION
cc32 (mp_no *x, mp_no *y, int p)
{
  int i;
  double a;
  mp_no mpt1, x2, gor, sum, mpk = {1, {1.0}};
  for (i = 1; i <= p; i++)
    mpk.d[i] = 0;

  __sqr (x, &x2, p);
  mpk.d[1] = 27.0;
  __mul (&oofac27, &mpk, &gor, p);
  __cpy (&gor, &sum, p);
  for (a = 26.0; a > 2.0; a -= 2.0)
    {
      mpk.d[1] = a * (a - 1.0);
      __mul (&gor, &mpk, &mpt1, p);
      __cpy (&mpt1, &gor, p);
      __mul (&x2, &sum, &mpt1, p);
      __sub (&gor, &mpt1, &sum, p);
    }
  __mul (&x2, &sum, y, p);
}

/* Compute both sin(x), cos(x) as Multi precision numbers.  */
void
SECTION
__c32 (mp_no *x, mp_no *y, mp_no *z, int p)
{
  mp_no u, t, t1, t2, c, s;
  int i;
  __cpy (x, &u, p);
  u.e = u.e - 1;
  cc32 (&u, &c, p);
  ss32 (&u, &s, p);
  for (i = 0; i < 24; i++)
    {
      __mul (&c, &s, &t, p);
      __sub (&s, &t, &t1, p);
      __add (&t1, &t1, &s, p);
      __sub (&__mptwo, &c, &t1, p);
      __mul (&t1, &c, &t2, p);
      __add (&t2, &t2, &c, p);
    }
  __sub (&__mpone, &c, y, p);
  __cpy (&s, z, p);
}

/* Receive double x and two double results of sin(x) and return result which is
   more accurate, computing sin(x) with multi precision routine c32.  */
double
SECTION
__sin32 (double x, double res, double res1)
{
  int p;
  mp_no a, b, c;
  p = 32;
  __dbl_mp (res, &a, p);
  __dbl_mp (0.5 * (res1 - res), &b, p);
  __add (&a, &b, &c, p);
  if (x > 0.8)
    {
      __sub (&hp, &c, &a, p);
      __c32 (&a, &b, &c, p);
    }
  else
    __c32 (&c, &a, &b, p);	/* b=sin(0.5*(res+res1))  */
  __dbl_mp (x, &c, p);		/* c = x  */
  __sub (&b, &c, &a, p);
  /* if a > 0 return min (res, res1), otherwise return max (res, res1).  */
  if ((a.d[0] > 0 && res >= res1) || (a.d[0] <= 0 && res <= res1))
    res = res1;
  LIBC_PROBE (slowasin, 2, &res, &x);
  return res;
}

/* Receive double x and two double results of cos(x) and return result which is
   more accurate, computing cos(x) with multi precision routine c32.  */
double
SECTION
__cos32 (double x, double res, double res1)
{
  int p;
  mp_no a, b, c;
  p = 32;
  __dbl_mp (res, &a, p);
  __dbl_mp (0.5 * (res1 - res), &b, p);
  __add (&a, &b, &c, p);
  if (x > 2.4)
    {
      __sub (&pi, &c, &a, p);
      __c32 (&a, &b, &c, p);
      b.d[0] = -b.d[0];
    }
  else if (x > 0.8)
    {
      __sub (&hp, &c, &a, p);
      __c32 (&a, &c, &b, p);
    }
  else
    __c32 (&c, &b, &a, p);	/* b=cos(0.5*(res+res1))  */
  __dbl_mp (x, &c, p);		/* c = x                  */
  __sub (&b, &c, &a, p);
  /* if a > 0 return max (res, res1), otherwise return min (res, res1).  */
  if ((a.d[0] > 0 && res <= res1) || (a.d[0] <= 0 && res >= res1))
    res = res1;
  LIBC_PROBE (slowacos, 2, &res, &x);
  return res;
}

/* Compute sin() of double-length number (X + DX) as Multi Precision number and
   return result as double.  If REDUCE_RANGE is true, X is assumed to be the
   original input and DX is ignored.  */
double
SECTION
__mpsin (double x, double dx, bool reduce_range)
{
  double y;
  mp_no a, b, c, s;
  int n;
  int p = 32;

  if (reduce_range)
    {
      n = __mpranred (x, &a, p);	/* n is 0, 1, 2 or 3.  */
      __c32 (&a, &c, &s, p);
    }
  else
    {
      n = -1;
      __dbl_mp (x, &b, p);
      __dbl_mp (dx, &c, p);
      __add (&b, &c, &a, p);
      if (x > 0.8)
        {
          __sub (&hp, &a, &b, p);
          __c32 (&b, &s, &c, p);
        }
      else
        __c32 (&a, &c, &s, p);	/* b = sin(x+dx)  */
    }

  /* Convert result based on which quarter of unit circle y is in.  */
  switch (n)
    {
    case 1:
      __mp_dbl (&c, &y, p);
      break;

    case 3:
      __mp_dbl (&c, &y, p);
      y = -y;
      break;

    case 2:
      __mp_dbl (&s, &y, p);
      y = -y;
      break;

    /* Quadrant not set, so the result must be sin (X + DX), which is also in
       S.  */
    case 0:
    default:
      __mp_dbl (&s, &y, p);
    }
  LIBC_PROBE (slowsin, 3, &x, &dx, &y);
  return y;
}

/* Compute cos() of double-length number (X + DX) as Multi Precision number and
   return result as double.  If REDUCE_RANGE is true, X is assumed to be the
   original input and DX is ignored.  */
double
SECTION
__mpcos (double x, double dx, bool reduce_range)
{
  double y;
  mp_no a, b, c, s;
  int n;
  int p = 32;

  if (reduce_range)
    {
      n = __mpranred (x, &a, p);	/* n is 0, 1, 2 or 3.  */
      __c32 (&a, &c, &s, p);
    }
  else
    {
      n = -1;
      __dbl_mp (x, &b, p);
      __dbl_mp (dx, &c, p);
      __add (&b, &c, &a, p);
      if (x > 0.8)
        {
          __sub (&hp, &a, &b, p);
          __c32 (&b, &s, &c, p);
        }
      else
        __c32 (&a, &c, &s, p);	/* a = cos(x+dx)     */
    }

  /* Convert result based on which quarter of unit circle y is in.  */
  switch (n)
    {
    case 1:
      __mp_dbl (&s, &y, p);
      y = -y;
      break;

    case 3:
      __mp_dbl (&s, &y, p);
      break;

    case 2:
      __mp_dbl (&c, &y, p);
      y = -y;
      break;

    /* Quadrant not set, so the result must be cos (X + DX), which is also
       stored in C.  */
    case 0:
    default:
      __mp_dbl (&c, &y, p);
    }
  LIBC_PROBE (slowcos, 3, &x, &dx, &y);
  return y;
}

/* Perform range reduction of a double number x into multi precision number y,
   such that y = x - n * pi / 2, abs (y) < pi / 4, n = 0, +-1, +-2, ...
   Return int which indicates in which quarter of circle x is.  */
int
SECTION
__mpranred (double x, mp_no *y, int p)
{
  number v;
  double t, xn;
  int i, k, n;
  mp_no a, b, c;

  if (fabs (x) < 2.8e14)
    {
      t = (x * hpinv.d + toint.d);
      xn = t - toint.d;
      v.d = t;
      n = v.i[LOW_HALF] & 3;
      __dbl_mp (xn, &a, p);
      __mul (&a, &hp, &b, p);
      __dbl_mp (x, &c, p);
      __sub (&c, &b, y, p);
      return n;
    }
  else
    {
      /* If x is very big more precision required.  */
      __dbl_mp (x, &a, p);
      a.d[0] = 1.0;
      k = a.e - 5;
      if (k < 0)
	k = 0;
      b.e = -k;
      b.d[0] = 1.0;
      for (i = 0; i < p; i++)
	b.d[i + 1] = toverp[i + k];
      __mul (&a, &b, &c, p);
      t = c.d[c.e];
      for (i = 1; i <= p - c.e; i++)
	c.d[i] = c.d[i + c.e];
      for (i = p + 1 - c.e; i <= p; i++)
	c.d[i] = 0;
      c.e = 0;
      if (c.d[1] >= HALFRAD)
	{
	  t += 1.0;
	  __sub (&c, &__mpone, &b, p);
	  __mul (&b, &hp, y, p);
	}
      else
	__mul (&c, &hp, y, p);
      n = (int) t;
      if (x < 0)
	{
	  y->d[0] = -y->d[0];
	  n = -n;
	}
      return (n & 3);
    }
}
