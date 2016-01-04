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
/**************************************************************************/
/*  MODULE_NAME:slowexp.c                                                 */
/*                                                                        */
/*  FUNCTION:slowexp                                                      */
/*                                                                        */
/*  FILES NEEDED:mpa.h                                                    */
/*               mpa.c mpexp.c                                            */
/*                                                                        */
/*Converting from double precision to Multi-precision and calculating     */
/* e^x                                                                    */
/**************************************************************************/
#include <math_private.h>

#include <stap-probe.h>

#ifndef USE_LONG_DOUBLE_FOR_MP
# include "mpa.h"
void __mpexp (mp_no *x, mp_no *y, int p);
#endif

#ifndef SECTION
# define SECTION
#endif

/*Converting from double precision to Multi-precision and calculating  e^x */
double
SECTION
__slowexp (double x)
{
#ifndef USE_LONG_DOUBLE_FOR_MP
  double w, z, res, eps = 3.0e-26;
  int p;
  mp_no mpx, mpy, mpz, mpw, mpeps, mpcor;

  /* Use the multiple precision __MPEXP function to compute the exponential
     First at 144 bits and if it is not accurate enough, at 768 bits.  */
  p = 6;
  __dbl_mp (x, &mpx, p);
  __mpexp (&mpx, &mpy, p);
  __dbl_mp (eps, &mpeps, p);
  __mul (&mpeps, &mpy, &mpcor, p);
  __add (&mpy, &mpcor, &mpw, p);
  __sub (&mpy, &mpcor, &mpz, p);
  __mp_dbl (&mpw, &w, p);
  __mp_dbl (&mpz, &z, p);
  if (w == z)
    {
      /* Track how often we get to the slow exp code plus
	 its input/output values.  */
      LIBC_PROBE (slowexp_p6, 2, &x, &w);
      return w;
    }
  else
    {
      p = 32;
      __dbl_mp (x, &mpx, p);
      __mpexp (&mpx, &mpy, p);
      __mp_dbl (&mpy, &res, p);

      /* Track how often we get to the uber-slow exp code plus
	 its input/output values.  */
      LIBC_PROBE (slowexp_p32, 2, &x, &res);
      return res;
    }
#else
  return (double) __ieee754_expl((long double)x);
#endif
}
