/* Compute sine and cosine of argument.
   Copyright (C) 1997, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997 and
		  Jakub Jelinek <jj@ultra.linux.cz>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <math.h>

#include "math_private.h"

/* Note: We should probably introduce __kernel_sincosl to speed things up,
   because __kernel_{cos,sin}l sometimes compute both sine and cosine.  */

void
__sincosl (long double x, long double *sinx, long double *cosx)
{
  int64_t ix;

  /* High word of x. */
  GET_LDOUBLE_MSW64 (ix, x);

  /* |x| ~< pi/4 */
  ix &= 0x7fffffffffffffffLL;
  if (ix <= 0x3ffe921fb54442d1LL)
    {
      *sinx = __kernel_sinl (x, 0.0, 0);
      *cosx = __kernel_cosl (x, 0.0);
    }
  else if (ix >= 0x7fff000000000000LL)
    {
      /* sin(Inf or NaN) is NaN */
      *sinx = *cosx = x - x;
    }
  else
    {
      /* Argument reduction needed.  */
      long double y[2];
      int n;

      n = __ieee754_rem_pio2l (x, y);
      switch (n & 3)
	{
	case 0:
	  *sinx = __kernel_sinl (y[0], y[1], 1);
	  *cosx = __kernel_cosl (y[0], y[1]);
	  break;
	case 1:
	  *sinx = __kernel_cosl (y[0], y[1]);
	  *cosx = -__kernel_sinl (y[0], y[1], 1);
	  break;
	case 2:
	  *sinx = -__kernel_sinl (y[0], y[1], 1);
	  *cosx = -__kernel_cosl (y[0], y[1]);
	  break;
	default:
	  *sinx = -__kernel_cosl (y[0], y[1]);
	  *cosx = __kernel_sinl (y[0], y[1], 1);
	  break;
	}
    }
}
weak_alias (__sincosl, sincosl)
