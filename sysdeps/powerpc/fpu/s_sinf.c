/* s_sinf.c -- float version of s_sin.c.
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

#include <errno.h>
#include "math.h"
#include "math_private.h"

static const float pio4 = 7.8539801e-1;

float
__sinf (float x)
{
  float y[2], z = 0.0;
  float ix;
  int32_t n;

  ix = __builtin_fabsf (x);

  /* |x| ~< pi/4 */
  if (ix <= pio4)
    {
      return __kernel_sinf (x, z, 0);
      /* sin(Inf or NaN) is NaN */
    }
  else if (isnanf (ix))
    {
      return x - x;
    }
  else if (isinff (ix))
    {
      __set_errno (EDOM);
      return x - x;
    }

  /* argument reduction needed */
  else
    {
      n = __ieee754_rem_pio2f (x, y);
      switch (n & 3)
	{
	case 0:
	  return __kernel_sinf (y[0], y[1], 1);
	case 1:
	  return __kernel_cosf (y[0], y[1]);
	case 2:
	  return -__kernel_sinf (y[0], y[1], 1);
	default:
	  return -__kernel_cosf (y[0], y[1]);
	}
    }
}

weak_alias (__sinf, sinf)
