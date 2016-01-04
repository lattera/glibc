/* k_sinf.c -- float version of k_sin.c
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

#include <float.h>
#include <math.h>
#include <fenv.h>
#include <math_private.h>


static const float twom27 =  7.4505806000e-09;
static const float half   =  5.0000000000e-01;
static const float S1     = -1.6666667163e-01;
static const float S2     =  8.3333337680e-03;
static const float S3     = -1.9841270114e-04;
static const float S4     =  2.7557314297e-06;
static const float S5     = -2.5050759689e-08;
static const float S6     =  1.5896910177e-10;


float
__kernel_sinf (float x, float y, int iy)
{
  float z, r, v;
  float ix;
  ix = __builtin_fabsf (x);
  if (ix < twom27)
    {				/* |x| < 2**-27 */
      if (ix < FLT_MIN && ix != 0.0f)
	__feraiseexcept (FE_UNDERFLOW|FE_INEXACT);
      else
	__feraiseexcept (FE_INEXACT);
      return x;
    }
  z = x * x;
  v = z * x;
  r = S2 + z * (S3 + z * (S4 + z * (S5 + z * S6)));
  if (iy == 0)
    return x + v * (S1 + z * r);
  else
    return x - ((z * (half * y - v * r) - y) - v * S1);
}
