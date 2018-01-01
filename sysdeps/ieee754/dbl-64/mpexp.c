/*
 * IBM Accurate Mathematical Library
 * written by International Business Machines Corp.
 * Copyright (C) 2001-2018 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  Lesser General Public License as published by
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
/*   MODULE_NAME:mpexp.c                                                 */
/*                                                                       */
/*   FUNCTIONS: mpexp                                                    */
/*                                                                       */
/*   FILES NEEDED: mpa.h endian.h mpexp.h                                */
/*                 mpa.c                                                 */
/*                                                                       */
/* Multi-Precision exponential function subroutine                       */
/*   (  for p >= 4, 2**(-55) <= abs(x) <= 1024     ).                    */
/*************************************************************************/

#include "endian.h"
#include "mpa.h"
#include <assert.h>

#ifndef SECTION
# define SECTION
#endif

/* Multi-Precision exponential function subroutine (for p >= 4,
   2**(-55) <= abs(x) <= 1024).  */
void
SECTION
__mpexp (mp_no *x, mp_no *y, int p)
{
  int i, j, k, m, m1, m2, n;
  mantissa_t b;
  static const int np[33] =
    {
      0, 0, 0, 0, 3, 3, 4, 4, 5, 4, 4, 5, 5, 5, 6, 6, 6, 6, 6, 6,
      6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8
    };

  static const int m1p[33] =
    {
      0, 0, 0, 0,
      17, 23, 23, 28,
      27, 38, 42, 39,
      43, 47, 43, 47,
      50, 54, 57, 60,
      64, 67, 71, 74,
      68, 71, 74, 77,
      70, 73, 76, 78,
      81
    };
  static const int m1np[7][18] =
    {
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 36, 48, 60, 72, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 24, 32, 40, 48, 56, 64, 72, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 17, 23, 29, 35, 41, 47, 53, 59, 65, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 23, 28, 33, 38, 42, 47, 52, 57, 62, 66, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 27, 0, 0, 39, 43, 47, 51, 55, 59, 63},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 43, 47, 50, 54}
    };
  mp_no mps, mpk, mpt1, mpt2;

  /* Choose m,n and compute a=2**(-m).  */
  n = np[p];
  m1 = m1p[p];
  b = X[1];
  m2 = 24 * EX;
  for (; b < HALFRAD; m2--)
    b *= 2;
  if (b == HALFRAD)
    {
      for (i = 2; i <= p; i++)
	{
	  if (X[i] != 0)
	    break;
	}
      if (i == p + 1)
	m2--;
    }

  m = m1 + m2;
  if (__glibc_unlikely (m <= 0))
    {
      /* The m1np array which is used to determine if we can reduce the
	 polynomial expansion iterations, has only 18 elements.  Besides,
	 numbers smaller than those required by p >= 18 should not come here
	 at all since the fast phase of exp returns 1.0 for anything less
	 than 2^-55.  */
      assert (p < 18);
      m = 0;
      for (i = n - 1; i > 0; i--, n--)
	if (m1np[i][p] + m2 > 0)
	  break;
    }

  /* Compute s=x*2**(-m). Put result in mps.  This is the range-reduced input
     that we will use to compute e^s.  For the final result, simply raise it
     to 2^m.  */
  __pow_mp (-m, &mpt1, p);
  __mul (x, &mpt1, &mps, p);

  /* Compute the Taylor series for e^s:

         1 + x/1! + x^2/2! + x^3/3! ...

     for N iterations.  We compute this as:

         e^x = 1 + (x * n!/1! + x^2 * n!/2! + x^3 * n!/3!) / n!
             = 1 + (x * (n!/1! + x * (n!/2! + x * (n!/3! + x ...)))) / n!

     k! is computed on the fly as KF and at the end of the polynomial loop, KF
     is n!, which can be used directly.  */
  __cpy (&mps, &mpt2, p);

  double kf = 1.0;

  /* Evaluate the rest.  The result will be in mpt2.  */
  for (k = n - 1; k > 0; k--)
    {
      /* n! / k! = n * (n - 1) ... * (n - k + 1) */
      kf *= k + 1;

      __dbl_mp (kf, &mpk, p);
      __add (&mpt2, &mpk, &mpt1, p);
      __mul (&mps, &mpt1, &mpt2, p);
    }
  __dbl_mp (kf, &mpk, p);
  __dvd (&mpt2, &mpk, &mpt1, p);
  __add (&__mpone, &mpt1, &mpt2, p);

  /* Raise polynomial value to the power of 2**m. Put result in y.  */
  for (k = 0, j = 0; k < m;)
    {
      __sqr (&mpt2, &mpt1, p);
      k++;
      if (k == m)
	{
	  j = 1;
	  break;
	}
      __sqr (&mpt1, &mpt2, p);
      k++;
    }
  if (j)
    __cpy (&mpt1, y, p);
  else
    __cpy (&mpt2, y, p);
  return;
}
