/* Double-precision floating point e^x.
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

/* How this works:
   The basic design here is from
   Shmuel Gal and Boris Bachelis, "An Accurate Elementary Mathematical
   Library for the IEEE Floating Point Standard", ACM Trans. Math. Soft.,
   17 (1), March 1991, pp. 26-45.

   The input value, x, is written as

   x = n * ln(2)_0 + t/512 + delta[t] + x + n * ln(2)_1

   where:
   - n is an integer, 1024 >= n >= -1075;
   - ln(2)_0 is the first 43 bits of ln(2), and ln(2)_1 is the remainder, so
     that |ln(2)_1| < 2^-32;
   - t is an integer, 177 >= t >= -177
   - delta is based on a table entry, delta[t] < 2^-28
   - x is whatever is left, |x| < 2^-10

   Then e^x is approximated as

   e^x = 2^n_1 ( 2^n_0 e^(t/512 + delta[t])
               + ( 2^n_0 e^(t/512 + delta[t])
                   * ( p(x + n * ln(2)_1)
                       - n*ln(2)_1
                       - n*ln(2)_1 * p(x + n * ln(2)_1) ) ) )

   where
   - p(x) is a polynomial approximating e(x)-1;
   - e^(t/512 + delta[t]) is obtained from a table;
   - n_1 + n_0 = n, so that |n_0| < DBL_MIN_EXP-1.

   If it happens that n_1 == 0 (this is the usual case), that multiplication
   is omitted.
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

extern const float __exp_deltatable[178];
extern const double __exp_atable[355] /* __attribute__((mode(DF))) */;

static const volatile double TWO1023 = 8.988465674311579539e+307;
static const volatile double TWOM1000 = 9.3326361850321887899e-302;

double
__ieee754_exp (double x)
{
  static const double himark = 709.7827128933840868;
  static const double lomark = -745.1332191019412221;
  /* Check for usual case.  */
  if (isless (x, himark) && isgreater (x, lomark))
    {
      static const double THREEp42 = 13194139533312.0;
      static const double THREEp51 = 6755399441055744.0;
      /* 1/ln(2).  */
      static const double M_1_LN2 = 1.442695040888963387;
      /* ln(2), part 1 */
      static const double M_LN2_0 = .6931471805598903302;
      /* ln(2), part 2 */
      static const double M_LN2_1 = 5.497923018708371155e-14;

      int tval, unsafe, n_i;
      double x22, n, t, dely, result;
      union ieee754_double ex2_u, scale_u;
      fenv_t oldenv;

      feholdexcept (&oldenv);
      fesetround (FE_TONEAREST);

      /* Calculate n.  */
      n = x * M_1_LN2 + THREEp51;
      n -= THREEp51;
      x = x - n*M_LN2_0;

      /* Calculate t/512.  */
      t = x + THREEp42;
      t -= THREEp42;
      x -= t;

      /* Compute tval = t.  */
      tval = (int) (t * 512.0);

      if (t >= 0)
	x -= __exp_deltatable[tval];
      else
	x += __exp_deltatable[-tval];

      /* Now, the variable x contains x + n*ln(2)_1.  */
      dely = n*M_LN2_1;

      /* Compute ex2 = 2^n_0 e^(t/512+delta[t]).  */
      ex2_u.d = __exp_atable[tval+177];
      n_i = (int)n;
      /* 'unsafe' is 1 iff n_1 != 0.  */
      unsafe = abs(n_i) >= -DBL_MIN_EXP - 1;
      ex2_u.ieee.exponent += n_i >> unsafe;

      /* Compute scale = 2^n_1.  */
      scale_u.d = 1.0;
      scale_u.ieee.exponent += n_i - (n_i >> unsafe);

      /* Approximate e^x2 - 1, using a fourth-degree polynomial,
	 with maximum error in [-2^-10-2^-28,2^-10+2^-28]
	 less than 4.9e-19.  */
      x22 = (((0.04166666898464281565
	       * x + 0.1666666766008501610)
	      * x + 0.499999999999990008)
	     * x + 0.9999999999999976685) * x;
      /* Allow for impact of dely.  */
      x22 -= dely + dely*x22;

      /* Return result.  */
      fesetenv (&oldenv);

      result = x22 * ex2_u.d + ex2_u.d;
      if (!unsafe)
	return result;
      else
	return result * scale_u.d;
    }
  /* Exceptional cases:  */
  else if (isless (x, himark))
    {
      if (__isinf (x))
	/* e^-inf == 0, with no error.  */
	return 0;
      else
	/* Underflow */
	return TWOM1000 * TWOM1000;
    }
  else
    /* Return x, if x is a NaN or Inf; or overflow, otherwise.  */
    return TWO1023*x;
}
