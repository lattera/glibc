/* k_rem_pio2f.c -- float version of e_rem_pio2.c
   Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Adhemerval Zanella <azanella@br.ibm.com>, 2011

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <math.h>

#include "math_private.h"
#include "s_float_bitwise.h"


static const float two_over_pi[] = {
  1.62000000e+02, 2.49000000e+02, 1.31000000e+02, 1.10000000e+02,
  7.80000000e+01, 6.80000000e+01, 2.10000000e+01, 4.10000000e+01,
  2.52000000e+02, 3.90000000e+01, 8.70000000e+01, 2.09000000e+02,
  2.45000000e+02, 5.20000000e+01, 2.21000000e+02, 1.92000000e+02,
  2.19000000e+02, 9.80000000e+01, 1.49000000e+02, 1.53000000e+02,
  6.00000000e+01, 6.70000000e+01, 1.44000000e+02, 6.50000000e+01,
  2.54000000e+02, 8.10000000e+01, 9.90000000e+01, 1.71000000e+02,
  2.22000000e+02, 1.87000000e+02, 1.97000000e+02, 9.70000000e+01,
  1.83000000e+02, 3.60000000e+01, 1.10000000e+02, 5.80000000e+01,
  6.60000000e+01, 7.70000000e+01, 2.10000000e+02, 2.24000000e+02,
  6.00000000e+00, 7.30000000e+01, 4.60000000e+01, 2.34000000e+02,
  9.00000000e+00, 2.09000000e+02, 1.46000000e+02, 2.80000000e+01,
  2.54000000e+02, 2.90000000e+01, 2.35000000e+02, 2.80000000e+01,
  1.77000000e+02, 4.10000000e+01, 1.67000000e+02, 6.20000000e+01,
  2.32000000e+02, 1.30000000e+02, 5.30000000e+01, 2.45000000e+02,
  4.60000000e+01, 1.87000000e+02, 6.80000000e+01, 1.32000000e+02,
  2.33000000e+02, 1.56000000e+02, 1.12000000e+02, 3.80000000e+01,
  1.80000000e+02, 9.50000000e+01, 1.26000000e+02, 6.50000000e+01,
  5.70000000e+01, 1.45000000e+02, 2.14000000e+02, 5.70000000e+01,
  1.31000000e+02, 8.30000000e+01, 5.70000000e+01, 2.44000000e+02,
  1.56000000e+02, 1.32000000e+02, 9.50000000e+01, 1.39000000e+02,
  1.89000000e+02, 2.49000000e+02, 4.00000000e+01, 5.90000000e+01,
  3.10000000e+01, 2.48000000e+02, 1.51000000e+02, 2.55000000e+02,
  2.22000000e+02, 5.00000000e+00, 1.52000000e+02, 1.50000000e+01,
  2.39000000e+02, 4.70000000e+01, 1.70000000e+01, 1.39000000e+02,
  9.00000000e+01, 1.00000000e+01, 1.09000000e+02, 3.10000000e+01,
  1.09000000e+02, 5.40000000e+01, 1.26000000e+02, 2.07000000e+02,
  3.90000000e+01, 2.03000000e+02, 9.00000000e+00, 1.83000000e+02,
  7.90000000e+01, 7.00000000e+01, 6.30000000e+01, 1.02000000e+02,
  1.58000000e+02, 9.50000000e+01, 2.34000000e+02, 4.50000000e+01,
  1.17000000e+02, 3.90000000e+01, 1.86000000e+02, 1.99000000e+02,
  2.35000000e+02, 2.29000000e+02, 2.41000000e+02, 1.23000000e+02,
  6.10000000e+01, 7.00000000e+00, 5.70000000e+01, 2.47000000e+02,
  1.38000000e+02, 8.20000000e+01, 1.46000000e+02, 2.34000000e+02,
  1.07000000e+02, 2.51000000e+02, 9.50000000e+01, 1.77000000e+02,
  3.10000000e+01, 1.41000000e+02, 9.30000000e+01, 8.00000000e+00,
  8.60000000e+01, 3.00000000e+00, 4.80000000e+01, 7.00000000e+01,
  2.52000000e+02, 1.23000000e+02, 1.07000000e+02, 1.71000000e+02,
  2.40000000e+02, 2.07000000e+02, 1.88000000e+02, 3.20000000e+01,
  1.54000000e+02, 2.44000000e+02, 5.40000000e+01, 2.90000000e+01,
  1.69000000e+02, 2.27000000e+02, 1.45000000e+02, 9.70000000e+01,
  9.40000000e+01, 2.30000000e+02, 2.70000000e+01, 8.00000000e+00,
  1.01000000e+02, 1.53000000e+02, 1.33000000e+02, 9.50000000e+01,
  2.00000000e+01, 1.60000000e+02, 1.04000000e+02, 6.40000000e+01,
  1.41000000e+02, 2.55000000e+02, 2.16000000e+02, 1.28000000e+02,
  7.70000000e+01, 1.15000000e+02, 3.90000000e+01, 4.90000000e+01,
  6.00000000e+00, 6.00000000e+00, 2.10000000e+01, 8.60000000e+01,
  2.02000000e+02, 1.15000000e+02, 1.68000000e+02, 2.01000000e+02,
  9.60000000e+01, 2.26000000e+02, 1.23000000e+02, 1.92000000e+02,
  1.40000000e+02, 1.07000000e+02
};


static const float PIo2[] = {
  1.5703125000e+00,		/* 0x3fc90000 */
  4.5776367188e-04,		/* 0x39f00000 */
  2.5987625122e-05,		/* 0x37da0000 */
  7.5437128544e-08,		/* 0x33a20000 */
  6.0026650317e-11,		/* 0x2e840000 */
  7.3896444519e-13,		/* 0x2b500000 */
  5.3845816694e-15,		/* 0x27c20000 */
  5.6378512969e-18,		/* 0x22d00000 */
  8.3009228831e-20,		/* 0x1fc40000 */
  3.2756352257e-22,		/* 0x1bc60000 */
  6.3331015649e-25,		/* 0x17440000 */
};


static const float zero  = 0.0000000000e+00;
static const float one   = 1.0000000000;
static const float twon8 = 3.9062500000e-03;
static const float two8  = 2.5600000000e+02;


int32_t
__fp_kernel_rem_pio2f (float *x, float *y, float e0, int32_t nx)
{
  int32_t jz, jx, jv, jp, jk, carry, n, iq[20], i, j, k, m, q0, ih, exp;
  float z, fw, f[20], fq[20], q[20];

  /* initialize jk */
  jp = jk = 9;

  /* determine jx,jv,q0, note that 3>q0 */
  jx = nx - 1;
  exp = __float_get_exp (e0) - 127;
  jv = (exp - 3) / 8;
  if (jv < 0)
    jv = 0;
  q0 = exp - 8 * (jv + 1);

  /* set up f[0] to f[jx+jk] where f[jx+jk] = two_over_pi[jv+jk] */
  j = jv - jx;
  m = jx + jk;
  for (i = 0; i <= m; i++, j++)
    f[i] = (j < 0) ? zero : two_over_pi[j];

  /* compute q[0],q[1],...q[jk] */
  for (i = 0; i <= jk; i++)
    {
      for (j = 0, fw = 0.0; j <= jx; j++)
	fw += x[j] * f[jx + i - j];
      q[i] = fw;
    }

  jz = jk;
recompute:
  /* distill q[] into iq[] reversingly */
  for (i = 0, j = jz, z = q[jz]; j > 0; i++, j--)
    {
      fw = __truncf (twon8 * z);
      iq[i] = (int32_t) (z - two8 * fw);
      z = q[j - 1] + fw;
    }

  /* compute n */
  z = __scalbnf (z, q0);	/* actual value of z */
  z -= 8.0 * __floorf (z * 0.125);	/* trim off integer >= 8 */
  n = (int32_t) z;
  z -= __truncf (z);
  ih = 0;
  if (q0 > 0)
    {				/* need iq[jz-1] to determine n */
      i = (iq[jz - 1] >> (8 - q0));
      n += i;
      iq[jz - 1] -= i << (8 - q0);
      ih = iq[jz - 1] >> (7 - q0);
    }
  else if (q0 == 0)
    ih = iq[jz - 1] >> 8;
  else if (z >= 0.5)
    ih = 2;

  if (ih > 0)
    {				/* q > 0.5 */
      n += 1;
      carry = 0;
      for (i = 0; i < jz; i++)
	{			/* compute 1-q */
	  j = iq[i];
	  if (carry == 0)
	    {
	      if (j != 0)
		{
		  carry = 1;
		  iq[i] = 0x100 - j;
		}
	    }
	  else
	    iq[i] = 0xff - j;
	}
      if (q0 > 0)
	{			/* rare case: chance is 1 in 12 */
	  switch (q0)
	    {
	    case 1:
	      iq[jz - 1] &= 0x7f;
	      break;
	    case 2:
	      iq[jz - 1] &= 0x3f;
	      break;
	    }
	}
      if (ih == 2)
	{
	  z = one - z;
	  if (carry != 0)
	    z -= __scalbnf (one, q0);
	}
    }

  /* check if recomputation is needed */
  if (z == zero)
    {
      j = 0;
      for (i = jz - 1; i >= jk; i--)
	j |= iq[i];
      if (j == 0)
	{			/* need recomputation */
	  for (k = 1; iq[jk - k] == 0; k++);	/* k = no. of terms needed */

	  for (i = jz + 1; i <= jz + k; i++)
	    {			/* add q[jz+1] to q[jz+k] */
	      f[jx + i] = two_over_pi[jv + i];
	      for (j = 0, fw = 0.0; j <= jx; j++)
		fw += x[j] * f[jx + i - j];
	      q[i] = fw;
	    }
	  jz += k;
	  goto recompute;
	}
    }

  /* chop off zero terms */
  if (z == 0.0)
    {
      jz -= 1;
      q0 -= 8;
      while (iq[jz] == 0)
	{
	  jz--;
	  q0 -= 8;
	}
    }
  else
    {				/* break z into 8-bit if necessary */
      z = __scalbnf (z, -q0);
      if (z >= two8)
	{
	  fw = __truncf (twon8 * z);
	  iq[jz] = (int32_t) (z - two8 * fw);
	  jz += 1;
	  q0 += 8;
	  iq[jz] = (int32_t) fw;
	}
      else
	iq[jz] = (int32_t) z;
    }

  /* convert integer "bit" chunk to floating-point value */
  fw = __scalbnf (one, q0);
  for (i = jz; i >= 0; i--)
    {
      q[i] = fw * (float) iq[i];
      fw *= twon8;
    }

  /* compute PIo2[0,...,jp]*q[jz,...,0] */
  for (i = jz; i >= 0; i--)
    {
      for (fw = 0.0, k = 0; k <= jp && k <= jz - i; k++)
	fw += PIo2[k] * q[i + k];
      fq[jz - i] = fw;
    }

  /* compress fq[] into y[] */
  fw = 0.0;
  for (i = jz; i >= 0; i--)
    fw += fq[i];
  y[0] = (ih == 0) ? fw : -fw;
  fw = fq[0] - fw;
  for (i = 1; i <= jz; i++)
    fw += fq[i];
  y[1] = (ih == 0) ? fw : -fw;

  return n & 7;
}
