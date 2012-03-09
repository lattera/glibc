/*
 * IBM Accurate Mathematical Library
 * written by International Business Machines Corp.
 * Copyright (C) 2001, 2006 Free Software Foundation
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
/* then routine converts x and y into multi-precision doubles and        */
/* recompute.                                                            */
/*************************************************************************/

#include "mpa.h"
#include <math_private.h>

void __mpexp (mp_no * x, mp_no * y, int p);
void __mplog (mp_no * x, mp_no * y, int p);
double ulog (double);
double __halfulp (double x, double y);

double
__slowpow (double x, double y, double z)
{
  double res, res1;
  long double ldw, ldz, ldpp;
  static const long double ldeps = 0x4.0p-96;

  res = __halfulp (x, y);	/* halfulp() returns -10 or x^y             */
  if (res >= 0)
    return res;			/* if result was really computed by halfulp */
  /*  else, if result was not really computed by halfulp */

  /* Compute pow as long double, 106 bits */
  ldz = __ieee754_logl ((long double) x);
  ldw = (long double) y *ldz;
  ldpp = __ieee754_expl (ldw);
  res = (double) (ldpp + ldeps);
  res1 = (double) (ldpp - ldeps);

  if (res != res1)		/* if result still not accurate enough */
    {				/* use mpa for higher persision.  */
      mp_no mpx, mpy, mpz, mpw, mpp, mpr, mpr1;
      static const mp_no eps = { -3, {1.0, 4.0} };
      int p;

      p = 10;			/*  p=precision 240 bits  */
      __dbl_mp (x, &mpx, p);
      __dbl_mp (y, &mpy, p);
      __dbl_mp (z, &mpz, p);
      __mplog (&mpx, &mpz, p);		/* log(x) = z   */
      __mul (&mpy, &mpz, &mpw, p);	/*  y * z =w    */
      __mpexp (&mpw, &mpp, p);		/*  e^w =pp     */
      __add (&mpp, &eps, &mpr, p);	/*  pp+eps =r   */
      __mp_dbl (&mpr, &res, p);
      __sub (&mpp, &eps, &mpr1, p);	/*  pp -eps =r1 */
      __mp_dbl (&mpr1, &res1, p);	/*  converting into double precision */
      if (res == res1)
	return res;

      /* if we get here result wasn't calculated exactly, continue for
         more exact calculation using 768 bits.  */
      p = 32;
      __dbl_mp (x, &mpx, p);
      __dbl_mp (y, &mpy, p);
      __dbl_mp (z, &mpz, p);
      __mplog (&mpx, &mpz, p);		/* log(c)=z  */
      __mul (&mpy, &mpz, &mpw, p);	/* y*z =w    */
      __mpexp (&mpw, &mpp, p);		/* e^w=pp    */
      __mp_dbl (&mpp, &res, p);		/* converting into double precision */
    }
  return res;
}
