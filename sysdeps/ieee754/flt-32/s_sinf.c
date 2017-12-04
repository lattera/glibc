/* Compute sine of argument.
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
#include <libm-alias-float.h>

#ifndef SINF
# define SINF_FUNC __sinf
#else
# define SINF_FUNC SINF
#endif

/* Chebyshev constants for cos, range -PI/4 - PI/4.  */
static const double C0 = -0x1.ffffffffe98aep-2;
static const double C1 =  0x1.55555545c50c7p-5;
static const double C2 = -0x1.6c16b348b6874p-10;
static const double C3 =  0x1.a00eb9ac43ccp-16;
static const double C4 = -0x1.23c97dd8844d7p-22;

/* Chebyshev constants for sin, range -PI/4 - PI/4.  */
static const double S0 = -0x1.5555555551cd9p-3;
static const double S1 =  0x1.1111110c2688bp-7;
static const double S2 = -0x1.a019f8b4bd1f9p-13;
static const double S3 =  0x1.71d7264e6b5b4p-19;
static const double S4 = -0x1.a947e1674b58ap-26;

/* Chebyshev constants for sin, range 2^-27 - 2^-5.  */
static const double SS0 = -0x1.555555543d49dp-3;
static const double SS1 =  0x1.110f475cec8c5p-7;

/* PI/2 with 98 bits of accuracy.  */
static const double PI_2_hi = -0x1.921fb544p+0;
static const double PI_2_lo = -0x1.0b4611a626332p-34;

static const double SMALL = 0x1p-50; /* 2^-50.  */
static const double inv_PI_4 = 0x1.45f306dc9c883p+0; /* 4/PI.  */

#define FLOAT_EXPONENT_SHIFT 23
#define FLOAT_EXPONENT_BIAS 127

static const double pio2_table[] = {
  0 * M_PI_2,
  1 * M_PI_2,
  2 * M_PI_2,
  3 * M_PI_2,
  4 * M_PI_2,
  5 * M_PI_2
};

static const double invpio4_table[] = {
  0x0p+0,
  0x1.45f306cp+0,
  0x1.c9c882ap-28,
  0x1.4fe13a8p-58,
  0x1.f47d4dp-85,
  0x1.bb81b6cp-112,
  0x1.4acc9ep-142,
  0x1.0e4107cp-169
};

static const int ones[] = { +1, -1 };

/* Compute the sine value using Chebyshev polynomials where
   THETA is the range reduced absolute value of the input
   and it is less than Pi/4,
   N is calculated as trunc(|x|/(Pi/4)) + 1 and it is used to decide
   whether a sine or cosine approximation is more accurate and
   SIGNBIT is used to add the correct sign after the Chebyshev
   polynomial is computed.  */
static inline float
reduced (const double theta, const unsigned long int n,
	 const unsigned long int signbit)
{
  double sx;
  const double theta2 = theta * theta;
  /* We are operating on |x|, so we need to add back the original
     signbit for sinf.  */
  int sign;
  /* Determine positive or negative primary interval.  */
  sign = ones[((n >> 2) & 1) ^ signbit];
  /* Are we in the primary interval of sin or cos?  */
  if ((n & 2) == 0)
    {
      /* Here sinf() is calculated using sin Chebyshev polynomial:
	x+x^3*(S0+x^2*(S1+x^2*(S2+x^2*(S3+x^2*S4)))).  */
      sx = S3 + theta2 * S4;     /* S3+x^2*S4.  */
      sx = S2 + theta2 * sx;     /* S2+x^2*(S3+x^2*S4).  */
      sx = S1 + theta2 * sx;     /* S1+x^2*(S2+x^2*(S3+x^2*S4)).  */
      sx = S0 + theta2 * sx;     /* S0+x^2*(S1+x^2*(S2+x^2*(S3+x^2*S4))).  */
      sx = theta + theta * theta2 * sx;
    }
  else
    {
     /* Here sinf() is calculated using cos Chebyshev polynomial:
	1.0+x^2*(C0+x^2*(C1+x^2*(C2+x^2*(C3+x^2*C4)))).  */
      sx = C3 + theta2 * C4;     /* C3+x^2*C4.  */
      sx = C2 + theta2 * sx;     /* C2+x^2*(C3+x^2*C4).  */
      sx = C1 + theta2 * sx;     /* C1+x^2*(C2+x^2*(C3+x^2*C4)).  */
      sx = C0 + theta2 * sx;     /* C0+x^2*(C1+x^2*(C2+x^2*(C3+x^2*C4))).  */
      sx = 1.0 + theta2 * sx;
    }

  /* Add in the signbit and assign the result.  */
  return sign * sx;
}

float
SINF_FUNC (float x)
{
  double cx;
  double theta = x;
  double abstheta = fabs (theta);
  /* If |x|< Pi/4.  */
  if (abstheta < M_PI_4)
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
      unsigned long int signbit = (x < 0);
      if (abstheta < 9 * M_PI_4)        /* |x| < 9*Pi/4.  */
	{
	  /* There are cases where FE_UPWARD rounding mode can
	     produce a result of abstheta * inv_PI_4 == 9,
	     where abstheta < 9pi/4, so the domain for
	     pio2_table must go to 5 (9 / 2 + 1).  */
	  unsigned long int n = (abstheta * inv_PI_4) + 1;
	  theta = abstheta - pio2_table[n / 2];
	  return reduced (theta, n, signbit);
	}
      else if (isless (abstheta, INFINITY))
	{
	  if (abstheta < 0x1p+23)     /* |x| < 2^23.  */
	    {
	      unsigned long int n = floor (abstheta * inv_PI_4) + 1.0;
	      double x = floor (n / 2.0);
	      theta = x * PI_2_lo + (x * PI_2_hi + abstheta);
	      /* Argument reduction needed.  */
	      return reduced (theta, n, signbit);
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
	          return reduced (e, l + 1, signbit);
	        }
	      else
		{
		  e += b;
		  e += c;
		  e += d;
		  if (e <= 1.0)
		    {
		      e *= M_PI_4;
		      return reduced (e, l + 1, signbit);
		    }
		  else
		    {
		      l++;
		      e -= 2.0;
		      e *= M_PI_4;
		      return reduced (e, l + 1, signbit);
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
