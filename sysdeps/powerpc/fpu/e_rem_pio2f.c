/* e_rem_pio2f.c -- float version of e_rem_pio2.c
   Copyright (C) 2011-2016 Free Software Foundation, Inc.
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

#include <math_private.h>
#include "s_float_bitwise.h"

/* defined in sysdeps/powerpc/fpu/k_rem_pio2f.c */
int __fp_kernel_rem_pio2f (float *x, float *y, float e0, int32_t nx);

/* __ieee754_rem_pio2f(x,y)
 *
 * return the remainder of x rem pi/2 in y[0]+y[1]
 */

static const float npio2_hw[] = {
  1.57077026e+00, 3.14154053e+00, 4.71228027e+00, 6.28308105e+00,
  7.85388184e+00, 9.42456055e+00, 1.09953613e+01, 1.25661621e+01,
  1.41369629e+01, 1.57077637e+01, 1.72783203e+01, 1.88491211e+01,
  2.04199219e+01, 2.19907227e+01, 2.35615234e+01, 2.51323242e+01,
  2.67031250e+01, 2.82739258e+01, 2.98447266e+01, 3.14155273e+01,
  3.29863281e+01, 3.45566406e+01, 3.61279297e+01, 3.76982422e+01,
  3.92695312e+01, 4.08398438e+01, 4.24111328e+01, 4.39814453e+01,
  4.55527344e+01, 4.71230469e+01, 4.86943359e+01, 5.02646484e+01
};


static const float zero  = 0.0000000000e+00;
static const float two8  = 2.5600000000e+02;

static const float half    = 5.0000000000e-01;
static const float invpio2 = 6.3661980629e-01;
static const float pio2_1  = 1.5707855225e+00;
static const float pio2_1t = 1.0804334124e-05;
static const float pio2_2  = 1.0804273188e-05;
static const float pio2_2t = 6.0770999344e-11;
static const float pio2_3  = 6.0770943833e-11;
static const float pio2_3t = 6.1232342629e-17;

static const float pio4     = 7.8539801e-01;
static const float pio3_4   = 2.3561945e+00;
static const float pio2_24b = 1.5707951e+00;
static const float pio2_2e7 = 2.0106054e+02;


int32_t
__ieee754_rem_pio2f (float x, float *y)
{
  float ax, z, n, r, w, t, e0;
  float tx[3];
  int32_t i, nx;

  ax = __builtin_fabsf (x);
  if (ax <= pio4)
    {
      y[0] = x;
      y[1] = 0;
      return 0;
    }
  if (ax < pio3_4)
    {
      if (x > 0)
	{
	  z = x - pio2_1;
	  if (!__float_and_test28 (ax, pio2_24b))
	    {
	      y[0] = z - pio2_1t;
	      y[1] = (z - y[0]) - pio2_1t;
	    }
	  else
	    {
	      z -= pio2_2;
	      y[0] = z - pio2_2t;
	      y[1] = (z - y[0]) - pio2_2t;
	    }
	  return 1;
	}
      else
	{
	  z = x + pio2_1;
	  if (!__float_and_test28 (ax, pio2_24b))
	    {
	      y[0] = z + pio2_1t;
	      y[1] = (z - y[0]) + pio2_1t;
	    }
	  else
	    {
	      z += pio2_2;
	      y[0] = z + pio2_2t;
	      y[1] = (z - y[0]) + pio2_2t;
	    }
	  return -1;
	}
    }
  if (ax <= pio2_2e7)
    {
      n = __floorf (ax * invpio2 + half);
      i = (int32_t) n;
      r = ax - n * pio2_1;
      w = n * pio2_1t;		/* 1st round good to 40 bit */
      if (i < 32 && !__float_and_test24 (ax, npio2_hw[i - 1]))
	{
	  y[0] = r - w;
	}
      else
	{
	  float i, j;
	  j = __float_and8 (ax);
	  y[0] = r - w;
	  i = __float_and8 (y[0]);
	  if (j / i > 256.0 || j / i < 3.9062500e-3)
	    {			/* 2nd iterations needed, good to 57 */
	      t = r;
	      w = n * pio2_2;
	      r = t - w;
	      w = n * pio2_2t - ((t - r) - w);
	      y[0] = r - w;
	      i = __float_and8 (y[0]);
	      if (j / i > 33554432 || j / i < 2.9802322e-8)
		{		/* 3rd iteration needed, 74 bits acc */
		  t = r;
		  w = n * pio2_3;
		  r = t - w;
		  w = n * pio2_3t - ((t - r) - w);
		  y[0] = r - w;
		}
	    }
	}
      y[1] = (r - y[0]) - w;
      if (x < 0)
	{
	  y[0] = -y[0];
	  y[1] = -y[1];
	  return -i;
	}
      else
	{
	  return i;
	}
    }

  /* all other (large) arguments */
  if (isnanf (x) || isinff (x))
    {
      y[0] = y[1] = x - x;
      return 0;
    }

  /* set z = scalbn(|x|,ilogb(x)-7) */
  e0 = __float_and8 (ax / 128.0);
  z = ax / e0;

  tx[0] = __floorf (z);
  z = (z - tx[0]) * two8;
  tx[1] = __floorf (z);
  z = (z - tx[1]) * two8;
  tx[2] = __floorf (z);

  nx = 3;
  while (tx[nx - 1] == zero)
    nx--;

  i = __fp_kernel_rem_pio2f (tx, y, e0, nx);
  if (x < 0)
    {
      y[0] = -y[0];
      y[1] = -y[1];
      return -i;
    }
  return i;
}
