/*
 * IBM Accurate Mathematical Library
 * written by International Business Machines Corp.
 * Copyright (C) 2001-2013 Free Software Foundation, Inc.
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
/*  MODULE_NAME: mpa.c                                                  */
/*                                                                      */
/*  FUNCTIONS:                                                          */
/*               mcr                                                    */
/*               acr                                                    */
/*               cpy                                                    */
/*               norm                                                   */
/*               denorm                                                 */
/*               mp_dbl                                                 */
/*               dbl_mp                                                 */
/*               add_magnitudes                                         */
/*               sub_magnitudes                                         */
/*               add                                                    */
/*               sub                                                    */
/*               mul                                                    */
/*               inv                                                    */
/*               dvd                                                    */
/*                                                                      */
/* Arithmetic functions for multiple precision numbers.                 */
/* Relative errors are bounded                                          */
/************************************************************************/


#include "endian.h"
#include "mpa.h"
#include <sys/param.h>

#ifndef SECTION
# define SECTION
#endif

#ifndef NO__CONST
const mp_no mpone = {1, {1.0, 1.0}};
const mp_no mptwo = {1, {1.0, 2.0}};
#endif

#ifndef NO___ACR
/* Compare mantissa of two multiple precision numbers regardless of the sign
   and exponent of the numbers.  */
static int
mcr (const mp_no *x, const mp_no *y, int p)
{
  int i;
  for (i = 1; i <= p; i++)
    {
      if (X[i] == Y[i])
	continue;
      else if (X[i] > Y[i])
	return 1;
      else
	return -1;
    }
  return 0;
}

/* Compare the absolute values of two multiple precision numbers.  */
int
__acr (const mp_no *x, const mp_no *y, int p)
{
  int i;

  if (X[0] == ZERO)
    {
      if (Y[0] == ZERO)
	i = 0;
      else
	i = -1;
    }
  else if (Y[0] == ZERO)
    i = 1;
  else
    {
      if (EX > EY)
	i = 1;
      else if (EX < EY)
	i = -1;
      else
	i = mcr (x, y, p);
    }

  return i;
}
#endif

#ifndef NO___CPY
/* Copy multiple precision number X into Y.  They could be the same
   number.  */
void
__cpy (const mp_no *x, mp_no *y, int p)
{
  EY = EX;
  for (int i = 0; i <= p; i++)
    Y[i] = X[i];
}
#endif

#ifndef NO___MP_DBL
/* Convert a multiple precision number *X into a double precision
   number *Y, normalized case  (|x| >= 2**(-1022))).  */
static void
norm (const mp_no *x, double *y, int p)
{
#define R  RADIXI
  int i;
  double a, c, u, v, z[5];
  if (p < 5)
    {
      if (p == 1)
	c = X[1];
      else if (p == 2)
	c = X[1] + R * X[2];
      else if (p == 3)
	c = X[1] + R * (X[2] + R * X[3]);
      else if (p == 4)
	c = (X[1] + R * X[2]) + R * R * (X[3] + R * X[4]);
    }
  else
    {
      for (a = ONE, z[1] = X[1]; z[1] < TWO23;)
	{
	  a *= TWO;
	  z[1] *= TWO;
	}

      for (i = 2; i < 5; i++)
	{
	  z[i] = X[i] * a;
	  u = (z[i] + CUTTER) - CUTTER;
	  if (u > z[i])
	    u -= RADIX;
	  z[i] -= u;
	  z[i - 1] += u * RADIXI;
	}

      u = (z[3] + TWO71) - TWO71;
      if (u > z[3])
	u -= TWO19;
      v = z[3] - u;

      if (v == TWO18)
	{
	  if (z[4] == ZERO)
	    {
	      for (i = 5; i <= p; i++)
		{
		  if (X[i] == ZERO)
		    continue;
		  else
		    {
		      z[3] += ONE;
		      break;
		    }
		}
	    }
	  else
	    z[3] += ONE;
	}

      c = (z[1] + R * (z[2] + R * z[3])) / a;
    }

  c *= X[0];

  for (i = 1; i < EX; i++)
    c *= RADIX;
  for (i = 1; i > EX; i--)
    c *= RADIXI;

  *y = c;
#undef R
}

/* Convert a multiple precision number *X into a double precision
   number *Y, Denormal case  (|x| < 2**(-1022))).  */
static void
denorm (const mp_no *x, double *y, int p)
{
  int i, k;
  double c, u, z[5];

#define R  RADIXI
  if (EX < -44 || (EX == -44 && X[1] < TWO5))
    {
      *y = ZERO;
      return;
    }

  if (p == 1)
    {
      if (EX == -42)
	{
	  z[1] = X[1] + TWO10;
	  z[2] = ZERO;
	  z[3] = ZERO;
	  k = 3;
	}
      else if (EX == -43)
	{
	  z[1] = TWO10;
	  z[2] = X[1];
	  z[3] = ZERO;
	  k = 2;
	}
      else
	{
	  z[1] = TWO10;
	  z[2] = ZERO;
	  z[3] = X[1];
	  k = 1;
	}
    }
  else if (p == 2)
    {
      if (EX == -42)
	{
	  z[1] = X[1] + TWO10;
	  z[2] = X[2];
	  z[3] = ZERO;
	  k = 3;
	}
      else if (EX == -43)
	{
	  z[1] = TWO10;
	  z[2] = X[1];
	  z[3] = X[2];
	  k = 2;
	}
      else
	{
	  z[1] = TWO10;
	  z[2] = ZERO;
	  z[3] = X[1];
	  k = 1;
	}
    }
  else
    {
      if (EX == -42)
	{
	  z[1] = X[1] + TWO10;
	  z[2] = X[2];
	  k = 3;
	}
      else if (EX == -43)
	{
	  z[1] = TWO10;
	  z[2] = X[1];
	  k = 2;
	}
      else
	{
	  z[1] = TWO10;
	  z[2] = ZERO;
	  k = 1;
	}
      z[3] = X[k];
    }

  u = (z[3] + TWO57) - TWO57;
  if (u > z[3])
    u -= TWO5;

  if (u == z[3])
    {
      for (i = k + 1; i <= p; i++)
	{
	  if (X[i] == ZERO)
	    continue;
	  else
	    {
	      z[3] += ONE;
	      break;
	    }
	}
    }

  c = X[0] * ((z[1] + R * (z[2] + R * z[3])) - TWO10);

  *y = c * TWOM1032;
#undef R
}

/* Convert multiple precision number *X into double precision number *Y.  The
   result is correctly rounded to the nearest/even.  */
void
__mp_dbl (const mp_no *x, double *y, int p)
{
  if (X[0] == ZERO)
    {
      *y = ZERO;
      return;
    }

  if (__glibc_likely (EX > -42 || (EX == -42 && X[1] >= TWO10)))
    norm (x, y, p);
  else
    denorm (x, y, p);
}
#endif

/* Get the multiple precision equivalent of X into *Y.  If the precision is too
   small, the result is truncated.  */
void
SECTION
__dbl_mp (double x, mp_no *y, int p)
{
  int i, n;
  double u;

  /* Sign.  */
  if (x == ZERO)
    {
      Y[0] = ZERO;
      return;
    }
  else if (x > ZERO)
    Y[0] = ONE;
  else
    {
      Y[0] = MONE;
      x = -x;
    }

  /* Exponent.  */
  for (EY = ONE; x >= RADIX; EY += ONE)
    x *= RADIXI;
  for (; x < ONE; EY -= ONE)
    x *= RADIX;

  /* Digits.  */
  n = MIN (p, 4);
  for (i = 1; i <= n; i++)
    {
      u = (x + TWO52) - TWO52;
      if (u > x)
	u -= ONE;
      Y[i] = u;
      x -= u;
      x *= RADIX;
    }
  for (; i <= p; i++)
    Y[i] = ZERO;
}

/* Add magnitudes of *X and *Y assuming that abs (*X) >= abs (*Y) > 0.  The
   sign of the sum *Z is not changed.  X and Y may overlap but not X and Z or
   Y and Z.  No guard digit is used.  The result equals the exact sum,
   truncated.  */
static void
SECTION
add_magnitudes (const mp_no *x, const mp_no *y, mp_no *z, int p)
{
  int i, j, k;

  EZ = EX;

  i = p;
  j = p + EY - EX;
  k = p + 1;

  if (j < 1)
    {
      __cpy (x, z, p);
      return;
    }
  else
    Z[k] = ZERO;

  for (; j > 0; i--, j--)
    {
      Z[k] += X[i] + Y[j];
      if (Z[k] >= RADIX)
	{
	  Z[k] -= RADIX;
	  Z[--k] = ONE;
	}
      else
	Z[--k] = ZERO;
    }

  for (; i > 0; i--)
    {
      Z[k] += X[i];
      if (Z[k] >= RADIX)
	{
	  Z[k] -= RADIX;
	  Z[--k] = ONE;
	}
      else
	Z[--k] = ZERO;
    }

  if (Z[1] == ZERO)
    {
      for (i = 1; i <= p; i++)
	Z[i] = Z[i + 1];
    }
  else
    EZ += ONE;
}

/* Subtract the magnitudes of *X and *Y assuming that abs (*x) > abs (*y) > 0.
   The sign of the difference *Z is not changed.  X and Y may overlap but not X
   and Z or Y and Z.  One guard digit is used.  The error is less than one
   ULP.  */
static void
SECTION
sub_magnitudes (const mp_no *x, const mp_no *y, mp_no *z, int p)
{
  int i, j, k;

  EZ = EX;

  if (EX == EY)
    {
      i = j = k = p;
      Z[k] = Z[k + 1] = ZERO;
    }
  else
    {
      j = EX - EY;
      if (j > p)
	{
	  __cpy (x, z, p);
	  return;
	}
      else
	{
	  i = p;
	  j = p + 1 - j;
	  k = p;
	  if (Y[j] > ZERO)
	    {
	      Z[k + 1] = RADIX - Y[j--];
	      Z[k] = MONE;
	    }
	  else
	    {
	      Z[k + 1] = ZERO;
	      Z[k] = ZERO;
	      j--;
	    }
	}
    }

  for (; j > 0; i--, j--)
    {
      Z[k] += (X[i] - Y[j]);
      if (Z[k] < ZERO)
	{
	  Z[k] += RADIX;
	  Z[--k] = MONE;
	}
      else
	Z[--k] = ZERO;
    }

  for (; i > 0; i--)
    {
      Z[k] += X[i];
      if (Z[k] < ZERO)
	{
	  Z[k] += RADIX;
	  Z[--k] = MONE;
	}
      else
	Z[--k] = ZERO;
    }

  for (i = 1; Z[i] == ZERO; i++);
  EZ = EZ - i + 1;
  for (k = 1; i <= p + 1;)
    Z[k++] = Z[i++];
  for (; k <= p;)
    Z[k++] = ZERO;
}

/* Add *X and *Y and store the result in *Z.  X and Y may overlap, but not X
   and Z or Y and Z.  One guard digit is used.  The error is less than one
   ULP.  */
void
SECTION
__add (const mp_no *x, const mp_no *y, mp_no *z, int p)
{
  int n;

  if (X[0] == ZERO)
    {
      __cpy (y, z, p);
      return;
    }
  else if (Y[0] == ZERO)
    {
      __cpy (x, z, p);
      return;
    }

  if (X[0] == Y[0])
    {
      if (__acr (x, y, p) > 0)
	{
	  add_magnitudes (x, y, z, p);
	  Z[0] = X[0];
	}
      else
	{
	  add_magnitudes (y, x, z, p);
	  Z[0] = Y[0];
	}
    }
  else
    {
      if ((n = __acr (x, y, p)) == 1)
	{
	  sub_magnitudes (x, y, z, p);
	  Z[0] = X[0];
	}
      else if (n == -1)
	{
	  sub_magnitudes (y, x, z, p);
	  Z[0] = Y[0];
	}
      else
	Z[0] = ZERO;
    }
}

/* Subtract *Y from *X and return the result in *Z.  X and Y may overlap but
   not X and Z or Y and Z.  One guard digit is used.  The error is less than
   one ULP.  */
void
SECTION
__sub (const mp_no *x, const mp_no *y, mp_no *z, int p)
{
  int n;

  if (X[0] == ZERO)
    {
      __cpy (y, z, p);
      Z[0] = -Z[0];
      return;
    }
  else if (Y[0] == ZERO)
    {
      __cpy (x, z, p);
      return;
    }

  if (X[0] != Y[0])
    {
      if (__acr (x, y, p) > 0)
	{
	  add_magnitudes (x, y, z, p);
	  Z[0] = X[0];
	}
      else
	{
	  add_magnitudes (y, x, z, p);
	  Z[0] = -Y[0];
	}
    }
  else
    {
      if ((n = __acr (x, y, p)) == 1)
	{
	  sub_magnitudes (x, y, z, p);
	  Z[0] = X[0];
	}
      else if (n == -1)
	{
	  sub_magnitudes (y, x, z, p);
	  Z[0] = -Y[0];
	}
      else
	Z[0] = ZERO;
    }
}

/* Multiply *X and *Y and store result in *Z.  X and Y may overlap but not X
   and Z or Y and Z.  For P in [1, 2, 3], the exact result is truncated to P
   digits.  In case P > 3 the error is bounded by 1.001 ULP.  */
void
SECTION
__mul (const mp_no *x, const mp_no *y, mp_no *z, int p)
{
  int i, j, k, k2;
  double u, zk;

  /* Is z=0?  */
  if (__glibc_unlikely (X[0] * Y[0] == ZERO))
    {
      Z[0] = ZERO;
      return;
    }

  /* Multiply, add and carry.  */
  k2 = (__glibc_unlikely (p < 3)) ? p + p : p + 3;
  zk = Z[k2] = ZERO;

  for (k = k2; k > p; k--)
    {
      for (i = k - p, j = p; i < p + 1; i++, j--)
	zk += X[i] * Y[j];

      u = (zk + CUTTER) - CUTTER;
      if (u > zk)
	u -= RADIX;
      Z[k] = zk - u;
      zk = u * RADIXI;
    }

  while (k > 1)
    {
      for (i = 1, j = k - 1; i < k; i++, j--)
	zk += X[i] * Y[j];

      u = (zk + CUTTER) - CUTTER;
      if (u > zk)
	u -= RADIX;
      Z[k] = zk - u;
      zk = u * RADIXI;
      k--;
    }
  Z[k] = zk;

  EZ = EX + EY;
  /* Is there a carry beyond the most significant digit?  */
  if (__glibc_unlikely (Z[1] == ZERO))
    {
      for (i = 1; i <= p; i++)
	Z[i] = Z[i + 1];
      EZ--;
    }

  Z[0] = X[0] * Y[0];
}

/* Invert *X and store in *Y.  Relative error bound:
   - For P = 2: 1.001 * R ^ (1 - P)
   - For P = 3: 1.063 * R ^ (1 - P)
   - For P > 3: 2.001 * R ^ (1 - P)

   *X = 0 is not permissible.  */
static void
SECTION
__inv (const mp_no *x, mp_no *y, int p)
{
  int i;
  double t;
  mp_no z, w;
  static const int np1[] =
    { 0, 0, 0, 0, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
  };

  __cpy (x, &z, p);
  z.e = 0;
  __mp_dbl (&z, &t, p);
  t = ONE / t;
  __dbl_mp (t, y, p);
  EY -= EX;

  for (i = 0; i < np1[p]; i++)
    {
      __cpy (y, &w, p);
      __mul (x, &w, y, p);
      __sub (&mptwo, y, &z, p);
      __mul (&w, &z, y, p);
    }
}

/* Divide *X by *Y and store result in *Z.  X and Y may overlap but not X and Z
   or Y and Z.  Relative error bound:
   - For P = 2: 2.001 * R ^ (1 - P)
   - For P = 3: 2.063 * R ^ (1 - P)
   - For P > 3: 3.001 * R ^ (1 - P)

   *X = 0 is not permissible.  */
void
SECTION
__dvd (const mp_no *x, const mp_no *y, mp_no *z, int p)
{
  mp_no w;

  if (X[0] == ZERO)
    Z[0] = ZERO;
  else
    {
      __inv (y, &w, p);
      __mul (x, &w, z, p);
    }
}
