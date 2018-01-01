/* Compute sine of argument.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <math.h>
#include <math_private.h>
#include <libm-alias-float.h>
#include "s_sincosf.h"

#ifndef SINF
# define SINF_FUNC __sinf
#else
# define SINF_FUNC SINF
#endif

float
SINF_FUNC (float x)
{
  double cx;
  double theta = x;
  double abstheta = fabs (theta);
  /* If |x|< Pi/4.  */
  if (isless (abstheta, M_PI_4))
    {
      if (abstheta >= 0x1p-5) /* |x| >= 2^-5.  */
	{
	  const double theta2 = theta * theta;
	  /* Chebyshev polynomial of the form for sin
	     x+x^3*(S0+x^2*(S1+x^2*(S2+x^2*(S3+x^2*S4)))).  */
	  cx = S3 + theta2 * S4;
	  cx = S2 + theta2 * cx;
	  cx = S1 + theta2 * cx;
	  cx = S0 + theta2 * cx;
	  cx = theta + theta * theta2 * cx;
	  return cx;
	}
      else if (abstheta >= 0x1p-27)     /* |x| >= 2^-27.  */
	{
	  /* A simpler Chebyshev approximation is close enough for this range:
	     for sin: x+x^3*(SS0+x^2*SS1).  */
	  const double theta2 = theta * theta;
	  cx = SS0 + theta2 * SS1;
	  cx = theta + theta * theta2 * cx;
	  return cx;
	}
      else
	{
	  /* Handle some special cases.  */
	  if (theta)
	    return theta - (theta * SMALL);
	  else
	    return theta;
	}
    }
  else                          /* |x| >= Pi/4.  */
    {
      unsigned int signbit = isless (x, 0);
      if (isless (abstheta, 9 * M_PI_4))        /* |x| < 9*Pi/4.  */
	{
	  /* There are cases where FE_UPWARD rounding mode can
	     produce a result of abstheta * inv_PI_4 == 9,
	     where abstheta < 9pi/4, so the domain for
	     pio2_table must go to 5 (9 / 2 + 1).  */
	  unsigned int n = (abstheta * inv_PI_4) + 1;
	  theta = abstheta - pio2_table[n / 2];
	  return reduced_sin (theta, n, signbit);
	}
      else if (isless (abstheta, INFINITY))
	{
	  if (abstheta < 0x1p+23)     /* |x| < 2^23.  */
	    {
	      unsigned int n = ((unsigned int) (abstheta * inv_PI_4)) + 1;
	      double x = n / 2;
	      theta = (abstheta - x * PI_2_hi) - x * PI_2_lo;
	      /* Argument reduction needed.  */
	      return reduced_sin (theta, n, signbit);
	    }
	  else                  /* |x| >= 2^23.  */
	    {
	      x = fabsf (x);
	      int exponent;
	      GET_FLOAT_WORD (exponent, x);
	      exponent
	        = (exponent >> FLOAT_EXPONENT_SHIFT) - FLOAT_EXPONENT_BIAS;
	      exponent += 3;
	      exponent /= 28;
	      double a = invpio4_table[exponent] * x;
	      double b = invpio4_table[exponent + 1] * x;
	      double c = invpio4_table[exponent + 2] * x;
	      double d = invpio4_table[exponent + 3] * x;
	      uint64_t l = a;
	      l &= ~0x7;
	      a -= l;
	      double e = a + b;
	      l = e;
	      e = a - l;
	      if (l & 1)
	        {
	          e -= 1.0;
	          e += b;
	          e += c;
	          e += d;
	          e *= M_PI_4;
	          return reduced_sin (e, l + 1, signbit);
	        }
	      else
		{
		  e += b;
		  e += c;
		  e += d;
		  if (e <= 1.0)
		    {
		      e *= M_PI_4;
		      return reduced_sin (e, l + 1, signbit);
		    }
		  else
		    {
		      l++;
		      e -= 2.0;
		      e *= M_PI_4;
		      return reduced_sin (e, l + 1, signbit);
		    }
		}
	    }
	}
      else
	{
	  int32_t ix;
	  /* High word of x.  */
	  GET_FLOAT_WORD (ix, abstheta);
	  /* Sin(Inf or NaN) is NaN.  */
	  if (ix == 0x7f800000)
	    __set_errno (EDOM);
	  return x - x;
	}
    }
}

#ifndef SINF
libm_alias_float (__sin, sin)
#endif
