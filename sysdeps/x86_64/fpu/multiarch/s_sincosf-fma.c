/* Compute sine and cosine of argument optimized with vector.
   Copyright (C) 2017 Free Software Foundation, Inc.
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
#include <x86intrin.h>
#include <libm-alias-float.h>
#include "s_sincosf.h"

#define SINCOSF __sincosf_fma

#ifndef SINCOSF
# define SINCOSF_FUNC __sincosf
#else
# define SINCOSF_FUNC SINCOSF
#endif

/* Chebyshev constants for sin and cos, range -PI/4 - PI/4.  */
static const __v2df V0 = { -0x1.5555555551cd9p-3, -0x1.ffffffffe98aep-2};
static const __v2df V1 = { 0x1.1111110c2688bp-7, 0x1.55555545c50c7p-5 };
static const __v2df V2 = { -0x1.a019f8b4bd1f9p-13, -0x1.6c16b348b6874p-10 };
static const __v2df V3 = { 0x1.71d7264e6b5b4p-19, 0x1.a00eb9ac43ccp-16 };
static const __v2df V4 = { -0x1.a947e1674b58ap-26, -0x1.23c97dd8844d7p-22 };

/* Chebyshev constants for sin and cos, range 2^-27 - 2^-5.  */
static const __v2df VC0 = { -0x1.555555543d49dp-3, -0x1.fffffff5cc6fdp-2 };
static const __v2df VC1 = { 0x1.110f475cec8c5p-7, 0x1.55514b178dac5p-5 };

static const __v2df v2ones = { 1.0, 1.0 };

/* Compute the sine and cosine values using Chebyshev polynomials where
   THETA is the range reduced absolute value of the input
   and it is less than Pi/4,
   N is calculated as trunc(|x|/(Pi/4)) + 1 and it is used to decide
   whether a sine or cosine approximation is more accurate and
   SIGNBIT is used to add the correct sign after the Chebyshev
   polynomial is computed.  */
static void
reduced_sincos (const double theta, const unsigned int n,
		const unsigned int signbit, float *sinx, float *cosx)
{
  __v2df v2x, v2sx, v2cx;
  const __v2df v2theta = { theta, theta };
  const __v2df v2theta2 = v2theta * v2theta;
  /* Here sinf() and cosf() are calculated using sin Chebyshev polynomial:
     x+x^3*(S0+x^2*(S1+x^2*(S2+x^2*(S3+x^2*S4)))).  */
  v2x = V3 + v2theta2 * V4;    /* S3+x^2*S4.  */
  v2x = V2 + v2theta2 * v2x;   /* S2+x^2*(S3+x^2*S4).  */
  v2x = V1 + v2theta2 * v2x;   /* S1+x^2*(S2+x^2*(S3+x^2*S4)).  */
  v2x = V0 + v2theta2 * v2x;   /* S0+x^2*(S1+x^2*(S2+x^2*(S3+x^2*S4))).  */
  v2x = v2theta2 * v2x;
  v2cx = v2ones + v2x;
  v2sx = v2theta + v2theta * v2x;
  /* We are operating on |x|, so we need to add back the original
     signbit for sinf.  */
  /* Determine positive or negative primary interval.  */
  /* Are we in the primary interval of sin or cos?  */
  if ((n & 2) == 0)
    {
      const __v2df v2sign =
	{
	  ones[((n >> 2) & 1) ^ signbit],
	  ones[((n + 2) >> 2) & 1]
	};
      v2cx[0] = v2sx[0];
      v2cx *= v2sign;
      __v4sf v4sx = _mm_cvtpd_ps (v2cx);
      *sinx = v4sx[0];
      *cosx = v4sx[1];
    }
  else
    {
      const __v2df v2sign =
	{
	  ones[((n + 2) >> 2) & 1],
	  ones[((n >> 2) & 1) ^ signbit]
	};
      v2cx[0] = v2sx[0];
      v2cx *= v2sign;
      __v4sf v4sx = _mm_cvtpd_ps (v2cx);
      *sinx = v4sx[1];
      *cosx = v4sx[0];
    }
}

void
SINCOSF_FUNC (float x, float *sinx, float *cosx)
{
  double theta = x;
  double abstheta = fabs (theta);
  uint32_t ix, xi;
  GET_FLOAT_WORD (xi, x);
  /* |x| */
  ix = xi & 0x7fffffff;
  /* If |x|< Pi/4.  */
  if (ix < 0x3f490fdb)
    {
      if (ix >= 0x3d000000) /* |x| >= 2^-5.  */
	{
	  __v2df v2x, v2sx, v2cx;
	  const __v2df v2theta = { theta, theta };
	  const __v2df v2theta2 = v2theta * v2theta;
	  /* Chebyshev polynomial of the form for sin and cos.  */
	  v2x = V3 + v2theta2 * V4;
	  v2x = V2 + v2theta2 * v2x;
	  v2x = V1 + v2theta2 * v2x;
	  v2x = V0 + v2theta2 * v2x;
	  v2x = v2theta2 * v2x;
	  v2cx = v2ones + v2x;
	  v2sx = v2theta + v2theta * v2x;
	  v2cx[0] = v2sx[0];
	  __v4sf v4sx = _mm_cvtpd_ps (v2cx);
	  *sinx = v4sx[0];
	  *cosx = v4sx[1];
	}
      else if (ix >= 0x32000000)     /* |x| >= 2^-27.  */
	{
	  /* A simpler Chebyshev approximation is close enough for this range:
	     for sin: x+x^3*(SS0+x^2*SS1)
	     for cos: 1.0+x^2*(CC0+x^3*CC1).  */
	  __v2df v2x, v2sx, v2cx;
	  const __v2df v2theta = { theta, theta };
	  const __v2df v2theta2 = v2theta * v2theta;
	  v2x = VC0 + v2theta * v2theta2 * VC1;
	  v2x = v2theta2 * v2x;
	  v2cx = v2ones + v2x;
	  v2sx = v2theta + v2theta * v2x;
	  v2cx[0] = v2sx[0];
	  __v4sf v4sx = _mm_cvtpd_ps (v2cx);
	  *sinx = v4sx[0];
	  *cosx = v4sx[1];
	}
      else
	{
	  /* Handle some special cases.  */
	  if (ix)
	    *sinx = theta - (theta * SMALL);
	  else
	    *sinx = theta;
	  *cosx = 1.0 - abstheta;
	}
    }
  else                          /* |x| >= Pi/4.  */
    {
      unsigned int signbit = xi >> 31;
      if (ix < 0x40e231d6) /* |x| < 9*Pi/4.  */
	{
	  /* There are cases where FE_UPWARD rounding mode can
	     produce a result of abstheta * inv_PI_4 == 9,
	     where abstheta < 9pi/4, so the domain for
	     pio2_table must go to 5 (9 / 2 + 1).  */
	  unsigned int n = (abstheta * inv_PI_4) + 1;
	  theta = abstheta - pio2_table[n / 2];
	  reduced_sincos (theta, n, signbit, sinx, cosx);
	}
      else if (ix < 0x7f800000)
	{
	  if (ix < 0x4b000000)     /* |x| < 2^23.  */
	    {
	      unsigned int n = ((unsigned int) (abstheta * inv_PI_4)) + 1;
	      double x = n / 2;
	      theta = (abstheta - x * PI_2_hi) - x * PI_2_lo;
	      /* Argument reduction needed.  */
	      reduced_sincos (theta, n, signbit, sinx, cosx);
	    }
	  else                  /* |x| >= 2^23.  */
	    {
	      x = fabsf (x);
	      int exponent
	        = (ix >> FLOAT_EXPONENT_SHIFT) - FLOAT_EXPONENT_BIAS;
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
		  reduced_sincos (e, l + 1, signbit, sinx, cosx);
	        }
	      else
		{
		  e += b;
		  e += c;
		  e += d;
		  if (e <= 1.0)
		    {
		      e *= M_PI_4;
		      reduced_sincos (e, l + 1, signbit, sinx, cosx);
		    }
		  else
		    {
		      l++;
		      e -= 2.0;
		      e *= M_PI_4;
		      reduced_sincos (e, l + 1, signbit, sinx, cosx);
		    }
		}
	    }
	}
      else
	{
	  if (ix == 0x7f800000)
	    __set_errno (EDOM);
	  /* sin/cos(Inf or NaN) is NaN.  */
	  *sinx = *cosx = x - x;
	}
    }
}

#ifndef SINCOSF
libm_alias_float (__sincos, sincos)
#endif
