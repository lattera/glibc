/* k_cosf.c -- float version of k_cos.c
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
#include <fenv.h>
#include <math_private.h>

static const float twom27   = 7.4505806e-09;
static const float dot3     = 3.0000001e-01;
static const float dot78125 = 7.8125000e-01;

static const float one =  1.0000000000e+00;
static const float C1  =  4.1666667908e-02;
static const float C2  = -1.3888889225e-03;
static const float C3  =  2.4801587642e-05;
static const float C4  = -2.7557314297e-07;
static const float C5  =  2.0875723372e-09;
static const float C6  = -1.1359647598e-11;

float
__kernel_cosf (float x, float y)
{
  float a, hz, z, r, qx;
  float ix;
  ix = __builtin_fabsf (x);
  if (ix < twom27)
    {				/* |x| < 2**-27 */
      __feraiseexcept (FE_INEXACT);
      return one;
    }
  z = x * x;
  r = z * (C1 + z * (C2 + z * (C3 + z * (C4 + z * (C5 + z * C6)))));
  if (ix < dot3)		/* if |x| < 0.3 */
    return one - ((float) 0.5 * z - (z * r - x * y));
  else
    {
      if (ix > dot78125)
	{			/* x > 0.78125 */
	  qx = (float) 0.28125;
	}
      else
	{
	  qx = ix / 4.0;
	}
      hz = (float) 0.5 *z - qx;
      a = one - qx;
      return a - (hz - (z * r - x * y));
    }
}
