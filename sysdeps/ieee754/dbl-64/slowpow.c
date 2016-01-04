/*
 * IBM Accurate Mathematical Library
 * written by International Business Machines Corp.
 * Copyright (C) 2001-2016 Free Software Foundation, Inc.
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
/*************************************************************************/
/* MODULE_NAME:slowpow.c                                                 */
/*                                                                       */
/* FUNCTION:slowpow                                                      */
/*                                                                       */
/*FILES NEEDED:mpa.h                                                     */
/*             mpa.c mpexp.c mplog.c halfulp.c                           */
/*                                                                       */
/* Given two IEEE double machine numbers y,x , routine  computes the     */
/* correctly  rounded (to nearest) value of x^y. Result calculated  by   */
/* multiplication (in halfulp.c) or if result isn't accurate enough      */
/* then routine converts x and y into multi-precision doubles     and    */
/* calls to mpexp routine                                                */
/*************************************************************************/

#include "mpa.h"
#include <math_private.h>

#include <stap-probe.h>

#ifndef SECTION
# define SECTION
#endif

void __mpexp (mp_no *x, mp_no *y, int p);
void __mplog (mp_no *x, mp_no *y, int p);
double ulog (double);
double __halfulp (double x, double y);

double
SECTION
__slowpow (double x, double y, double z)
{
  double res, res1;
  mp_no mpx, mpy, mpz, mpw, mpp, mpr, mpr1;
  static const mp_no eps = {-3, {1.0, 4.0}};
  int p;

  /* __HALFULP returns -10 or X^Y.  */
  res = __halfulp (x, y);

  /* Return if the result was computed by __HALFULP.  */
  if (res >= 0)
    return res;

  /* Compute pow as long double.  This is currently only used by powerpc, where
     one may get 106 bits of accuracy.  */
#ifdef USE_LONG_DOUBLE_FOR_MP
  long double ldw, ldz, ldpp;
  static const long double ldeps = 0x4.0p-96;

  ldz = __ieee754_logl ((long double) x);
  ldw = (long double) y *ldz;
  ldpp = __ieee754_expl (ldw);
  res = (double) (ldpp + ldeps);
  res1 = (double) (ldpp - ldeps);

  /* Return the result if it is accurate enough.  */
  if (res == res1)
    return res;
#endif

  /* Or else, calculate using multiple precision.  P = 10 implies accuracy of
     240 bits accuracy, since MP_NO has a radix of 2^24.  */
  p = 10;
  __dbl_mp (x, &mpx, p);
  __dbl_mp (y, &mpy, p);
  __dbl_mp (z, &mpz, p);

  /* z = x ^ y
     log (z) = y * log (x)
     z = exp (y * log (x))  */
  __mplog (&mpx, &mpz, p);
  __mul (&mpy, &mpz, &mpw, p);
  __mpexp (&mpw, &mpp, p);

  /* Add and subtract EPS to ensure that the result remains unchanged, i.e. we
     have last bit accuracy.  */
  __add (&mpp, &eps, &mpr, p);
  __mp_dbl (&mpr, &res, p);
  __sub (&mpp, &eps, &mpr1, p);
  __mp_dbl (&mpr1, &res1, p);
  if (res == res1)
    {
      /* Track how often we get to the slow pow code plus
	 its input/output values.  */
      LIBC_PROBE (slowpow_p10, 4, &x, &y, &z, &res);
      return res;
    }

  /* If we don't, then we repeat using a higher precision.  768 bits of
     precision ought to be enough for anybody.  */
  p = 32;
  __dbl_mp (x, &mpx, p);
  __dbl_mp (y, &mpy, p);
  __dbl_mp (z, &mpz, p);
  __mplog (&mpx, &mpz, p);
  __mul (&mpy, &mpz, &mpw, p);
  __mpexp (&mpw, &mpp, p);
  __mp_dbl (&mpp, &res, p);

  /* Track how often we get to the uber-slow pow code plus
     its input/output values.  */
  LIBC_PROBE (slowpow_p32, 4, &x, &y, &z, &res);

  return res;
}
