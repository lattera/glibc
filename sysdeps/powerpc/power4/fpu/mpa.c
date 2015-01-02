
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

/* Define __mul and __sqr and use the rest from generic code.  */
#define NO__MUL
#define NO__SQR

#include <sysdeps/ieee754/dbl-64/mpa.c>

/* Multiply *X and *Y and store result in *Z.  X and Y may overlap but not X
   and Z or Y and Z.  For P in [1, 2, 3], the exact result is truncated to P
   digits.  In case P > 3 the error is bounded by 1.001 ULP.  */
void
__mul (const mp_no *x, const mp_no *y, mp_no *z, int p)
{
  long i, i1, i2, j, k, k2;
  long p2 = p;
  double u, zk, zk2;

  /* Is z=0?  */
  if (__glibc_unlikely (X[0] * Y[0] == 0))
    {
      Z[0] = 0;
      return;
    }

  /* Multiply, add and carry */
  k2 = (p2 < 3) ? p2 + p2 : p2 + 3;
  zk = Z[k2] = 0;
  for (k = k2; k > 1;)
    {
      if (k > p2)
	{
	  i1 = k - p2;
	  i2 = p2 + 1;
	}
      else
	{
	  i1 = 1;
	  i2 = k;
	}
#if 1
      /* Rearrange this inner loop to allow the fmadd instructions to be
         independent and execute in parallel on processors that have
         dual symmetrical FP pipelines.  */
      if (i1 < (i2 - 1))
	{
	  /* Make sure we have at least 2 iterations.  */
	  if (((i2 - i1) & 1L) == 1L)
	    {
	      /* Handle the odd iterations case.  */
	      zk2 = x->d[i2 - 1] * y->d[i1];
	    }
	  else
	    zk2 = 0.0;
	  /* Do two multiply/adds per loop iteration, using independent
	     accumulators; zk and zk2.  */
	  for (i = i1, j = i2 - 1; i < i2 - 1; i += 2, j -= 2)
	    {
	      zk += x->d[i] * y->d[j];
	      zk2 += x->d[i + 1] * y->d[j - 1];
	    }
	  zk += zk2;		/* Final sum.  */
	}
      else
	{
	  /* Special case when iterations is 1.  */
	  zk += x->d[i1] * y->d[i1];
	}
#else
      /* The original code.  */
      for (i = i1, j = i2 - 1; i < i2; i++, j--)
	zk += X[i] * Y[j];
#endif

      u = (zk + CUTTER) - CUTTER;
      if (u > zk)
	u -= RADIX;
      Z[k] = zk - u;
      zk = u * RADIXI;
      --k;
    }
  Z[k] = zk;

  int e = EX + EY;
  /* Is there a carry beyond the most significant digit?  */
  if (Z[1] == 0)
    {
      for (i = 1; i <= p2; i++)
	Z[i] = Z[i + 1];
      e--;
    }

  EZ = e;
  Z[0] = X[0] * Y[0];
}

/* Square *X and store result in *Y.  X and Y may not overlap.  For P in
   [1, 2, 3], the exact result is truncated to P digits.  In case P > 3 the
   error is bounded by 1.001 ULP.  This is a faster special case of
   multiplication.  */
void
__sqr (const mp_no *x, mp_no *y, int p)
{
  long i, j, k, ip;
  double u, yk;

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
      double yk2 = 0.0;
      long lim = k / 2;

      if (k % 2 == 0)
        {
	  yk += X[lim] * X[lim];
	  lim--;
	}

      /* In __mul, this loop (and the one within the next while loop) run
         between a range to calculate the mantissa as follows:

         Z[k] = X[k] * Y[n] + X[k+1] * Y[n-1] ... + X[n-1] * Y[k+1]
		+ X[n] * Y[k]

         For X == Y, we can get away with summing halfway and doubling the
	 result.  For cases where the range size is even, the mid-point needs
	 to be added separately (above).  */
      for (i = k - p, j = p; i <= lim; i++, j--)
	yk2 += X[i] * X[j];

      yk += 2.0 * yk2;

      u = (yk + CUTTER) - CUTTER;
      if (u > yk)
	u -= RADIX;
      Y[k--] = yk - u;
      yk = u * RADIXI;
    }

  while (k > 1)
    {
      double yk2 = 0.0;
      long lim = k / 2;

      if (k % 2 == 0)
        {
	  yk += X[lim] * X[lim];
	  lim--;
	}

      /* Likewise for this loop.  */
      for (i = 1, j = k - 1; i <= lim; i++, j--)
	yk2 += X[i] * X[j];

      yk += 2.0 * yk2;

      u = (yk + CUTTER) - CUTTER;
      if (u > yk)
	u -= RADIX;
      Y[k--] = yk - u;
      yk = u * RADIXI;
    }
  Y[k] = yk;

  /* Squares are always positive.  */
  Y[0] = 1.0;

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
