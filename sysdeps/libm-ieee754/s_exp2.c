/* Double-precision floating point 2^x.
   Copyright (C) 1997 Free Software Foundation, Inc.
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
   It has been slightly modified to compute 2^x instead of e^x.
   */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <float.h>
#include <ieee754.h>
#include <math.h>
#include <fenv.h>
#include <inttypes.h>
#include <math_private.h>

#include "t_exp2.h"

static const volatile double TWO1000 = 1.071508607186267320948e+301;
static const volatile double TWOM1000 = 9.3326361850321887899e-302;

double
__ieee754_exp2 (double x)
{
  static const uint32_t a_inf = 0x7f800000;
  /* Check for usual case.  */
  if (isless (x, (double) DBL_MAX_EXP)
      && isgreater (x, (double) (DBL_MIN_EXP - 1)))
    {
      static const float TWO43 = 8796093022208.0;
      int tval;
      double rx, x22;
      union ieee754_double ex2_u;
      fenv_t oldenv;

      feholdexcept (&oldenv);
      fesetround (FE_TONEAREST);

      /* 1. Argument reduction.
	 Choose integers ex, -256 <= t < 256, and some real
	 -1/1024 <= x1 <= 1024 so that
	 x = ex + t/512 + x1.

	 First, calculate rx = ex + t/512.  */
      if (x >= 0)
	{
	  rx = x + TWO43;
	  rx -= TWO43;
	}
      else
	{
	  rx = x - TWO43;
	  rx += TWO43;
	}
      x -= rx;  /* Compute x=x1. */
      /* Compute tval = (ex*512 + t)+256.
	 Now, t = (tval mod 512)-256 and ex=tval/512  [that's mod, NOT %; and
	 /-round-to-nearest not the usual c integer /].  */
      tval = (int) (rx * 512.0 + 256.0);

      /* 2. Adjust for accurate table entry.
	 Find e so that
	 x = ex + t/512 + e + x2
	 where -1e6 < e < 1e6, and
	 (double)(2^(t/512+e))
	 is accurate to one part in 2^-64.  */

      /* 'tval & 511' is the same as 'tval%512' except that it's always
	 positive.
	 Compute x = x2.  */
      x -= exp2_deltatable[tval & 511];

      /* 3. Compute ex2 = 2^(t/512+e+ex).  */
      ex2_u.d = exp2_accuratetable[tval & 511];
      ex2_u.ieee.exponent += tval >> 9;

      /* 4. Approximate 2^x2 - 1, using a fourth-degree polynomial,
	 2^x2 ~= sum(k=0..4 | (x2 * ln(2))^k / k! ) +
	 so
	 2^x2 - 1 ~= sum(k=1..4 | (x2 * ln(2))^k / k! )
	 with error less than 2^(1/1024) * (x2 * ln(2))^5 / 5! < 1.2e-18.  */

      x22 = (((.0096181291076284772
	       * x + .055504108664821580)
	      * x + .240226506959100712)
	     * x + .69314718055994531) * ex2_u.d;

      /* 5. Return (2^x2-1) * 2^(t/512+e+ex) + 2^(t/512+e+ex).  */
      fesetenv (&oldenv);

      /* Need to check: does this set FE_INEXACT correctly? */
      return x22 * x + ex2_u.d;
    }
  /* 2^inf == inf, with no error.  */
  else if (x == *(const float *) &a_inf)
    return x;
  /* Check for overflow.  */
  else if (isgreaterequal (x, (double) DBL_MAX_EXP))
    return TWO1000 * TWO1000;
  /* And underflow (including -inf).  */
  else if (isless (x, (double) (DBL_MIN_EXP - DBL_MANT_DIG)))
    return TWOM1000 * TWOM1000;
  /* Maybe the result needs to be a denormalised number...  */
  else if (!isnan (x))
    return __ieee754_exp2 (x + 1000.0) * TWOM1000;
  else /* isnan(x) */
    return x + x;
}
