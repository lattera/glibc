/* Single-precision floating point 2^x.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Geoffrey Keating <geoffk@ozemail.com.au>

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

/* The basic design here is from
   Shmuel Gal and Boris Bachelis, "An Accurate Elementary Mathematical
   Library for the IEEE Floating Point Standard", ACM Trans. Math. Soft.,
   17 (1), March 1991, pp. 26-45.
   It has been slightly modified to compute 2^x instead of e^x, and for
   single-precision.
   */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <float.h>
#include <ieee754.h>
#include <math.h>
#include <fenv.h>
#include <inttypes.h>
#include <math_private.h>

#include "t_exp2f.h"

static const volatile float TWOM100 = 7.88860905e-31;
static const volatile float huge = 1e+30;

float
__ieee754_exp2f (float x)
{
  static const uint32_t a_inf = 0x7f800000;
  /* Check for usual case.  */
  if (isless (x, (float) FLT_MAX_EXP)
      && isgreater (x, (float) (FLT_MIN_EXP - 1)))
    {
      static const float TWO16 = 65536.0;
      int tval;
      float rx, x22;
      union ieee754_float ex2_u;
      fenv_t oldenv;

      feholdexcept (&oldenv);
#ifdef FE_TONEAREST
      /* If we don't have this, it's too bad.  */
      fesetround (FE_TONEAREST);
#endif

      /* 1. Argument reduction.
	 Choose integers ex, -128 <= t < 128, and some real
	 -1/512 <= x1 <= 1/512 so that
	 x = ex + t/512 + x1.

	 First, calculate rx = ex + t/256.  */
      if (x >= 0)
	{
	  rx = x + TWO16;
	  rx -= TWO16;
	}
      else
	{
	  rx = x - TWO16;
	  rx += TWO16;
	}
      x -= rx;  /* Compute x=x1. */
      /* Compute tval = (ex*256 + t)+128.
	 Now, t = (tval mod 256)-128 and ex=tval/256  [that's mod, NOT %; and
	 /-round-to-nearest not the usual c integer /].  */
      tval = (int) (rx * 256.0f + 128.0f);

      /* 2. Adjust for accurate table entry.
	 Find e so that
	 x = ex + t/256 + e + x2
	 where -7e-4 < e < 7e-4, and
	 (float)(2^(t/256+e))
	 is accurate to one part in 2^-64.  */

      /* 'tval & 255' is the same as 'tval%256' except that it's always
	 positive.
	 Compute x = x2.  */
      x -= exp2_deltatable[tval & 255];

      /* 3. Compute ex2 = 2^(t/255+e+ex).  */
      ex2_u.f = exp2_accuratetable[tval & 255];
      ex2_u.ieee.exponent += tval >> 8;

      /* 4. Approximate 2^x2 - 1, using a second-degree polynomial,
	 2^x2 ~= sum(k=0..2 | (x2 * ln(2))^k / k! ) +
	 so
	 2^x2 - 1 ~= sum(k=1..4 | (x2 * ln(2))^k / k! )
	 with error less than 2^(1/512+7e-4) * (x2 * ln(2))^3 / 3! < 1.2e-18.  */

      x22 = (.240226507f * x + .6931471806f) * ex2_u.f;

      /* 5. Return (2^x2-1) * 2^(t/512+e+ex) + 2^(t/512+e+ex).  */
      fesetenv (&oldenv);

      /* Need to check: does this set FE_INEXACT correctly? */
      return x22 * x + ex2_u.f;
    }
  /* 2^inf == inf, with no error.  */
  else if (x == *(const float *)&a_inf)
    {
      return x;
    }
  /* Check for overflow.  */
  else if (isgreaterequal (x, (float) FLT_MAX_EXP))
    return huge * huge;
  /* And underflow (including -inf).  */
  else if (isless (x, (float) (FLT_MIN_EXP - FLT_MANT_DIG)))
    return TWOM100 * TWOM100;
  /* Maybe the result needs to be a denormalised number...  */
  else if (!isnan (x))
    return __ieee754_exp2f (x + 100.0) * TWOM100;
  else /* isnan(x) */
    return x + x;
}
