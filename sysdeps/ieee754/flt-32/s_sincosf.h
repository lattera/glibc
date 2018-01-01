/* Used by sinf, cosf and sincosf functions.
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

/* Chebyshev constants for cos, range 2^-27 - 2^-5.  */
static const double CC0 = -0x1.fffffff5cc6fdp-2;
static const double CC1 =  0x1.55514b178dac5p-5;

/* PI/2 with 98 bits of accuracy.  */
static const double PI_2_hi = 0x1.921fb544p+0;
static const double PI_2_lo = 0x1.0b4611a626332p-34;

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

static const double ones[] = { 1.0, -1.0 };

/* Compute the sine value using Chebyshev polynomials where
   THETA is the range reduced absolute value of the input
   and it is less than Pi/4,
   N is calculated as trunc(|x|/(Pi/4)) + 1 and it is used to decide
   whether a sine or cosine approximation is more accurate and
   SIGNBIT is used to add the correct sign after the Chebyshev
   polynomial is computed.  */
static inline float
reduced_sin (const double theta, const unsigned int n,
	 const unsigned int signbit)
{
  double sx;
  const double theta2 = theta * theta;
  /* We are operating on |x|, so we need to add back the original
     signbit for sinf.  */
  double sign;
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

/* Compute the cosine value using Chebyshev polynomials where
   THETA is the range reduced absolute value of the input
   and it is less than Pi/4,
   N is calculated as trunc(|x|/(Pi/4)) + 1 and it is used to decide
   whether a sine or cosine approximation is more accurate and
   the sign of the result.  */
static inline float
reduced_cos (double theta, unsigned int n)
{
  double sign, cx;
  const double theta2 = theta * theta;

  /* Determine positive or negative primary interval.  */
  n += 2;
  sign = ones[(n >> 2) & 1];

  /* Are we in the primary interval of sin or cos?  */
  if ((n & 2) == 0)
    {
      /* Here cosf() is calculated using sin Chebyshev polynomial:
	x+x^3*(S0+x^2*(S1+x^2*(S2+x^2*(S3+x^2*S4)))).  */
      cx = S3 + theta2 * S4;
      cx = S2 + theta2 * cx;
      cx = S1 + theta2 * cx;
      cx = S0 + theta2 * cx;
      cx = theta + theta * theta2 * cx;
    }
  else
    {
     /* Here cosf() is calculated using cos Chebyshev polynomial:
	1.0+x^2*(C0+x^2*(C1+x^2*(C2+x^2*(C3+x^2*C4)))).  */
      cx = C3 + theta2 * C4;
      cx = C2 + theta2 * cx;
      cx = C1 + theta2 * cx;
      cx = C0 + theta2 * cx;
      cx = 1. + theta2 * cx;
    }
  return sign * cx;
}
