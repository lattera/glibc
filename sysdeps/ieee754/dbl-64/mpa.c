/*
 * IBM Accurate Mathematical Library
 * written by International Business Machines Corp.
 * Copyright (C) 2001-2015 Free Software Foundation, Inc.
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
#include <alloca.h>

#ifndef SECTION
# define SECTION
#endif

#ifndef NO__CONST
const mp_no __mpone = { 1, { 1.0, 1.0 } };
const mp_no __mptwo = { 1, { 1.0, 2.0 } };
#endif

#ifndef NO___ACR
/* Compare mantissa of two multiple precision numbers regardless of the sign
   and exponent of the numbers.  */
static int
mcr (const mp_no *x, const mp_no *y, int p)
{
  long i;
  long p2 = p;
  for (i = 1; i <= p2; i++)
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
  long i;

  if (X[0] == 0)
    {
      if (Y[0] == 0)
	i = 0;
      else
	i = -1;
    }
  else if (Y[0] == 0)
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
  long i;

  EY = EX;
  for (i = 0; i <= p; i++)
    Y[i] = X[i];
}
#endif

#ifndef NO___MP_DBL
/* Convert a multiple precision number *X into a double precision
   number *Y, normalized case  (|x| >= 2**(-1022))).  */
static void
norm (const mp_no *x, double *y, int p)
{
# define R RADIXI
  long i;
  double c;
  mantissa_t a, u, v, z[5];
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
      for (a = 1, z[1] = X[1]; z[1] < TWO23; )
	{
	  a *= 2;
	  z[1] *= 2;
	}

      for (i = 2; i < 5; i++)
	{
	  mantissa_store_t d, r;
	  d = X[i] * (mantissa_store_t) a;
	  DIV_RADIX (d, r);
	  z[i] = r;
	  z[i - 1] += d;
	}

      u = ALIGN_DOWN_TO (z[3], TWO19);
      v = z[3] - u;

      if (v == TWO18)
	{
	  if (z[4] == 0)
	    {
	      for (i = 5; i <= p; i++)
		{
		  if (X[i] == 0)
		    continue;
		  else
		    {
		      z[3] += 1;
		      break;
		    }
		}
	    }
	  else
	    z[3] += 1;
	}

      c = (z[1] + R * (z[2] + R * z[3])) / a;
    }

  c *= X[0];

  for (i = 1; i < EX; i++)
    c *= RADIX;
  for (i = 1; i > EX; i--)
    c *= RADIXI;

  *y = c;
# undef R
}

/* Convert a multiple precision number *X into a double precision
   number *Y, Denormal case  (|x| < 2**(-1022))).  */
static void
denorm (const mp_no *x, double *y, int p)
{
  long i, k;
  long p2 = p;
  double c;
  mantissa_t u, z[5];

# define R RADIXI
  if (EX < -44 || (EX == -44 && X[1] < TWO5))
    {
      *y = 0;
      return;
    }

  if (p2 == 1)
    {
      if (EX == -42)
	{
	  z[1] = X[1] + TWO10;
	  z[2] = 0;
	  z[3] = 0;
	  k = 3;
	}
      else if (EX == -43)
	{
	  z[1] = TWO10;
	  z[2] = X[1];
	  z[3] = 0;
	  k = 2;
	}
      else
	{
	  z[1] = TWO10;
	  z[2] = 0;
	  z[3] = X[1];
	  k = 1;
	}
    }
  else if (p2 == 2)
    {
      if (EX == -42)
	{
	  z[1] = X[1] + TWO10;
	  z[2] = X[2];
	  z[3] = 0;
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
	  z[2] = 0;
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
	  z[2] = 0;
	  k = 1;
	}
      z[3] = X[k];
    }

  u = ALIGN_DOWN_TO (z[3], TWO5);

  if (u == z[3])
    {
      for (i = k + 1; i <= p2; i++)
	{
	  if (X[i] == 0)
	    continue;
	  else
	    {
	      z[3] += 1;
	      break;
	    }
	}
    }

  c = X[0] * ((z[1] + R * (z[2] + R * z[3])) - TWO10);

  *y = c * TWOM1032;
# undef R
}

/* Convert multiple precision number *X into double precision number *Y.  The
   result is correctly rounded to the nearest/even.  */
void
__mp_dbl (const mp_no *x, double *y, int p)
{
  if (X[0] == 0)
    {
      *y = 0;
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
  long i, n;
  long p2 = p;

  /* Sign.  */
  if (x == 0)
    {
      Y[0] = 0;
      return;
    }
  else if (x > 0)
    Y[0] = 1;
  else
    {
      Y[0] = -1;
      x = -x;
    }

  /* Exponent.  */
  for (EY = 1; x >= RADIX; EY += 1)
    x *= RADIXI;
  for (; x < 1; EY -= 1)
    x *= RADIX;

  /* Digits.  */
  n = MIN (p2, 4);
  for (i = 1; i <= n; i++)
    {
      INTEGER_OF (x, Y[i]);
      x *= RADIX;
    }
  for (; i <= p2; i++)
    Y[i] = 0;
}

/* Add magnitudes of *X and *Y assuming that abs (*X) >= abs (*Y) > 0.  The
   sign of the sum *Z is not changed.  X and Y may overlap but not X and Z or
   Y and Z.  No guard digit is used.  The result equals the exact sum,
   truncated.  */
static void
SECTION
add_magnitudes (const mp_no *x, const mp_no *y, mp_no *z, int p)
{
  long i, j, k;
  long p2 = p;
  mantissa_t zk;

  EZ = EX;

  i = p2;
  j = p2 + EY - EX;
  k = p2 + 1;

  if (__glibc_unlikely (j < 1))
    {
      __cpy (x, z, p);
      return;
    }

  zk = 0;

  for (; j > 0; i--, j--)
    {
      zk += X[i] + Y[j];
      if (zk >= RADIX)
	{
	  Z[k--] = zk - RADIX;
	  zk = 1;
	}
      else
	{
	  Z[k--] = zk;
	  zk = 0;
	}
    }

  for (; i > 0; i--)
    {
      zk += X[i];
      if (zk >= RADIX)
	{
	  Z[k--] = zk - RADIX;
	  zk = 1;
	}
      else
	{
	  Z[k--] = zk;
	  zk = 0;
	}
    }

  if (zk == 0)
    {
      for (i = 1; i <= p2; i++)
	Z[i] = Z[i + 1];
    }
  else
    {
      Z[1] = zk;
      EZ += 1;
    }
}

/* Subtract the magnitudes of *X and *Y assuming that abs (*x) > abs (*y) > 0.
   The sign of the difference *Z is not changed.  X and Y may overlap but not X
   and Z or Y and Z.  One guard digit is used.  The error is less than one
   ULP.  */
static void
SECTION
sub_magnitudes (const mp_no *x, const mp_no *y, mp_no *z, int p)
{
  long i, j, k;
  long p2 = p;
  mantissa_t zk;

  EZ = EX;
  i = p2;
  j = p2 + EY - EX;
  k = p2;

  /* Y is too small compared to X, copy X over to the result.  */
  if (__glibc_unlikely (j < 1))
    {
      __cpy (x, z, p);
      return;
    }

  /* The relevant least significant digit in Y is non-zero, so we factor it in
     to enhance accuracy.  */
  if (j < p2 && Y[j + 1] > 0)
    {
      Z[k + 1] = RADIX - Y[j + 1];
      zk = -1;
    }
  else
    zk = Z[k + 1] = 0;

  /* Subtract and borrow.  */
  for (; j > 0; i--, j--)
    {
      zk += (X[i] - Y[j]);
      if (zk < 0)
	{
	  Z[k--] = zk + RADIX;
	  zk = -1;
	}
      else
	{
	  Z[k--] = zk;
	  zk = 0;
	}
    }

  /* We're done with digits from Y, so it's just digits in X.  */
  for (; i > 0; i--)
    {
      zk += X[i];
      if (zk < 0)
	{
	  Z[k--] = zk + RADIX;
	  zk = -1;
	}
      else
	{
	  Z[k--] = zk;
	  zk = 0;
	}
    }

  /* Normalize.  */
  for (i = 1; Z[i] == 0; i++)
    ;
  EZ = EZ - i + 1;
  for (k = 1; i <= p2 + 1; )
    Z[k++] = Z[i++];
  for (; k <= p2; )
    Z[k++] = 0;
}

/* Add *X and *Y and store the result in *Z.  X and Y may overlap, but not X
   and Z or Y and Z.  One guard digit is used.  The error is less than one
   ULP.  */
void
SECTION
__add (const mp_no *x, const mp_no *y, mp_no *z, int p)
{
  int n;

  if (X[0] == 0)
    {
      __cpy (y, z, p);
      return;
    }
  else if (Y[0] == 0)
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
	Z[0] = 0;
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

  if (X[0] == 0)
    {
      __cpy (y, z, p);
      Z[0] = -Z[0];
      return;
    }
  else if (Y[0] == 0)
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
	Z[0] = 0;
    }
}

#ifndef NO__MUL
/* Multiply *X and *Y and store result in *Z.  X and Y may overlap but not X
   and Z or Y and Z.  For P in [1, 2, 3], the exact result is truncated to P
   digits.  In case P > 3 the error is bounded by 1.001 ULP.  */
void
SECTION
__mul (const mp_no *x, const mp_no *y, mp_no *z, int p)
{
  long i, j, k, ip, ip2;
  long p2 = p;
  mantissa_store_t zk;
  const mp_no *a;
  mantissa_store_t *diag;

  /* Is z=0?  */
  if (__glibc_unlikely (X[0] * Y[0] == 0))
    {
      Z[0] = 0;
      return;
    }

  /* We need not iterate through all X's and Y's since it's pointless to
     multiply zeroes.  Here, both are zero...  */
  for (ip2 = p2; ip2 > 0; ip2--)
    if (X[ip2] != 0 || Y[ip2] != 0)
      break;

  a = X[ip2] != 0 ? y : x;

  /* ... and here, at least one of them is still zero.  */
  for (ip = ip2; ip > 0; ip--)
    if (a->d[ip] != 0)
      break;

  /* The product looks like this for p = 3 (as an example):


				a1    a2    a3
		 x		b1    b2    b3
		 -----------------------------
			     a1*b3 a2*b3 a3*b3
		       a1*b2 a2*b2 a3*b2
		 a1*b1 a2*b1 a3*b1

     So our K needs to ideally be P*2, but we're limiting ourselves to P + 3
     for P >= 3.  We compute the above digits in two parts; the last P-1
     digits and then the first P digits.  The last P-1 digits are a sum of
     products of the input digits from P to P-k where K is 0 for the least
     significant digit and increases as we go towards the left.  The product
     term is of the form X[k]*X[P-k] as can be seen in the above example.

     The first P digits are also a sum of products with the same product term,
     except that the sum is from 1 to k.  This is also evident from the above
     example.

     Another thing that becomes evident is that only the most significant
     ip+ip2 digits of the result are non-zero, where ip and ip2 are the
     'internal precision' of the input numbers, i.e. digits after ip and ip2
     are all 0.  */

  k = (__glibc_unlikely (p2 < 3)) ? p2 + p2 : p2 + 3;

  while (k > ip + ip2 + 1)
    Z[k--] = 0;

  zk = 0;

  /* Precompute sums of diagonal elements so that we can directly use them
     later.  See the next comment to know we why need them.  */
  diag = alloca (k * sizeof (mantissa_store_t));
  mantissa_store_t d = 0;
  for (i = 1; i <= ip; i++)
    {
      d += X[i] * (mantissa_store_t) Y[i];
      diag[i] = d;
    }
  while (i < k)
    diag[i++] = d;

  while (k > p2)
    {
      long lim = k / 2;

      if (k % 2 == 0)
	/* We want to add this only once, but since we subtract it in the sum
	   of products above, we add twice.  */
	zk += 2 * X[lim] * (mantissa_store_t) Y[lim];

      for (i = k - p2, j = p2; i < j; i++, j--)
	zk += (X[i] + X[j]) * (mantissa_store_t) (Y[i] + Y[j]);

      zk -= diag[k - 1];

      DIV_RADIX (zk, Z[k]);
      k--;
    }

  /* The real deal.  Mantissa digit Z[k] is the sum of all X[i] * Y[j] where i
     goes from 1 -> k - 1 and j goes the same range in reverse.  To reduce the
     number of multiplications, we halve the range and if k is an even number,
     add the diagonal element X[k/2]Y[k/2].  Through the half range, we compute
     X[i] * Y[j] as (X[i] + X[j]) * (Y[i] + Y[j]) - X[i] * Y[i] - X[j] * Y[j].

     This reduction tells us that we're summing two things, the first term
     through the half range and the negative of the sum of the product of all
     terms of X and Y in the full range.  i.e.

     SUM(X[i] * Y[i]) for k terms.  This is precalculated above for each k in
     a single loop so that it completes in O(n) time and can hence be directly
     used in the loop below.  */
  while (k > 1)
    {
      long lim = k / 2;

      if (k % 2 == 0)
	/* We want to add this only once, but since we subtract it in the sum
	   of products above, we add twice.  */
        zk += 2 * X[lim] * (mantissa_store_t) Y[lim];

      for (i = 1, j = k - 1; i < j; i++, j--)
	zk += (X[i] + X[j]) * (mantissa_store_t) (Y[i] + Y[j]);

      zk -= diag[k - 1];

      DIV_RADIX (zk, Z[k]);
      k--;
    }
  Z[k] = zk;

  /* Get the exponent sum into an intermediate variable.  This is a subtle
     optimization, where given enough registers, all operations on the exponent
     happen in registers and the result is written out only once into EZ.  */
  int e = EX + EY;

  /* Is there a carry beyond the most significant digit?  */
  if (__glibc_unlikely (Z[1] == 0))
    {
      for (i = 1; i <= p2; i++)
	Z[i] = Z[i + 1];
      e--;
    }

  EZ = e;
  Z[0] = X[0] * Y[0];
}
#endif

#ifndef NO__SQR
/* Square *X and store result in *Y.  X and Y may not overlap.  For P in
   [1, 2, 3], the exact result is truncated to P digits.  In case P > 3 the
   error is bounded by 1.001 ULP.  This is a faster special case of
   multiplication.  */
void
SECTION
__sqr (const mp_no *x, mp_no *y, int p)
{
  long i, j, k, ip;
  mantissa_store_t yk;

  /* Is z=0?  */
  if (__glibc_unlikely (X[0] == 0))
    {
      Y[0] = 0;
      return;
    }

  /* We need not iterate through all X's since it's pointless to
     multiply zeroes.  */
  for (ip = p; ip > 0; ip--)
    if (X[ip] != 0)
      break;

  k = (__glibc_unlikely (p < 3)) ? p + p : p + 3;

  while (k > 2 * ip + 1)
    Y[k--] = 0;

  yk = 0;

  while (k > p)
    {
      mantissa_store_t yk2 = 0;
      long lim = k / 2;

      if (k % 2 == 0)
	yk += X[lim] * (mantissa_store_t) X[lim];

      /* In __mul, this loop (and the one within the next while loop) run
         between a range to calculate the mantissa as follows:

         Z[k] = X[k] * Y[n] + X[k+1] * Y[n-1] ... + X[n-1] * Y[k+1]
		+ X[n] * Y[k]

         For X == Y, we can get away with summing halfway and doubling the
	 result.  For cases where the range size is even, the mid-point needs
	 to be added separately (above).  */
      for (i = k - p, j = p; i < j; i++, j--)
	yk2 += X[i] * (mantissa_store_t) X[j];

      yk += 2 * yk2;

      DIV_RADIX (yk, Y[k]);
      k--;
    }

  while (k > 1)
    {
      mantissa_store_t yk2 = 0;
      long lim = k / 2;

      if (k % 2 == 0)
	yk += X[lim] * (mantissa_store_t) X[lim];

      /* Likewise for this loop.  */
      for (i = 1, j = k - 1; i < j; i++, j--)
	yk2 += X[i] * (mantissa_store_t) X[j];

      yk += 2 * yk2;

      DIV_RADIX (yk, Y[k]);
      k--;
    }
  Y[k] = yk;

  /* Squares are always positive.  */
  Y[0] = 1;

  /* Get the exponent sum into an intermediate variable.  This is a subtle
     optimization, where given enough registers, all operations on the exponent
     happen in registers and the result is written out only once into EZ.  */
  int e = EX * 2;

  /* Is there a carry beyond the most significant digit?  */
  if (__glibc_unlikely (Y[1] == 0))
    {
      for (i = 1; i <= p; i++)
	Y[i] = Y[i + 1];
      e--;
    }

  EY = e;
}
#endif

/* Invert *X and store in *Y.  Relative error bound:
   - For P = 2: 1.001 * R ^ (1 - P)
   - For P = 3: 1.063 * R ^ (1 - P)
   - For P > 3: 2.001 * R ^ (1 - P)

   *X = 0 is not permissible.  */
static void
SECTION
__inv (const mp_no *x, mp_no *y, int p)
{
  long i;
  double t;
  mp_no z, w;
  static const int np1[] =
    { 0, 0, 0, 0, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
  };

  __cpy (x, &z, p);
  z.e = 0;
  __mp_dbl (&z, &t, p);
  t = 1 / t;
  __dbl_mp (t, y, p);
  EY -= EX;

  for (i = 0; i < np1[p]; i++)
    {
      __cpy (y, &w, p);
      __mul (x, &w, y, p);
      __sub (&__mptwo, y, &z, p);
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

  if (X[0] == 0)
    Z[0] = 0;
  else
    {
      __inv (y, &w, p);
      __mul (x, &w, z, p);
    }
}
