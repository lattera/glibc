/* Compute x * y + z as ternary operation.
   Copyright (C) 2010 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2010.

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

#include <float.h>
#include <math.h>
#include <fenv.h>
#include <ieee754.h>

/* This implementation uses rounding to odd to avoid problems with
   double rounding.  See a paper by Boldo and Melquiond:
   http://www.lri.fr/~melquion/doc/08-tc.pdf  */

long double
__fmal (long double x, long double y, long double z)
{
  union ieee854_long_double u, v, w;
  int adjust = 0;
  u.d = x;
  v.d = y;
  w.d = z;
  if (__builtin_expect (u.ieee.exponent + v.ieee.exponent
			>= 0x7fff + IEEE854_LONG_DOUBLE_BIAS
			   - LDBL_MANT_DIG, 0)
      || __builtin_expect (u.ieee.exponent >= 0x7fff - LDBL_MANT_DIG, 0)
      || __builtin_expect (v.ieee.exponent >= 0x7fff - LDBL_MANT_DIG, 0)
      || __builtin_expect (w.ieee.exponent >= 0x7fff - LDBL_MANT_DIG, 0)
      || __builtin_expect (u.ieee.exponent + v.ieee.exponent
			   <= IEEE854_LONG_DOUBLE_BIAS + LDBL_MANT_DIG, 0))
    {
      /* If z is Inf, but x and y are finite, the result should be
	 z rather than NaN.  */
      if (w.ieee.exponent == 0x7fff
	  && u.ieee.exponent != 0x7fff
          && v.ieee.exponent != 0x7fff)
	return (z + x) + y;
      /* If x or y or z is Inf/NaN, or if fma will certainly overflow,
	 or if x * y is less than half of LDBL_DENORM_MIN,
	 compute as x * y + z.  */
      if (u.ieee.exponent == 0x7fff
	  || v.ieee.exponent == 0x7fff
	  || w.ieee.exponent == 0x7fff
	  || u.ieee.exponent + v.ieee.exponent
	     > 0x7fff + IEEE854_LONG_DOUBLE_BIAS
	  || u.ieee.exponent + v.ieee.exponent
	     < IEEE854_LONG_DOUBLE_BIAS - LDBL_MANT_DIG - 2)
	return x * y + z;
      if (u.ieee.exponent + v.ieee.exponent
	  >= 0x7fff + IEEE854_LONG_DOUBLE_BIAS - LDBL_MANT_DIG)
	{
	  /* Compute 1p-113 times smaller result and multiply
	     at the end.  */
	  if (u.ieee.exponent > v.ieee.exponent)
	    u.ieee.exponent -= LDBL_MANT_DIG;
	  else
	    v.ieee.exponent -= LDBL_MANT_DIG;
	  /* If x + y exponent is very large and z exponent is very small,
	     it doesn't matter if we don't adjust it.  */
	  if (w.ieee.exponent > LDBL_MANT_DIG)
	    w.ieee.exponent -= LDBL_MANT_DIG;
	  adjust = 1;
	}
      else if (w.ieee.exponent >= 0x7fff - LDBL_MANT_DIG)
	{
	  /* Similarly.
	     If z exponent is very large and x and y exponents are
	     very small, it doesn't matter if we don't adjust it.  */
	  if (u.ieee.exponent > v.ieee.exponent)
	    {
	      if (u.ieee.exponent > LDBL_MANT_DIG)
		u.ieee.exponent -= LDBL_MANT_DIG;
	    }
	  else if (v.ieee.exponent > LDBL_MANT_DIG)
	    v.ieee.exponent -= LDBL_MANT_DIG;
	  w.ieee.exponent -= LDBL_MANT_DIG;
	  adjust = 1;
	}
      else if (u.ieee.exponent >= 0x7fff - LDBL_MANT_DIG)
	{
	  u.ieee.exponent -= LDBL_MANT_DIG;
	  if (v.ieee.exponent)
	    v.ieee.exponent += LDBL_MANT_DIG;
	  else
	    v.d *= 0x1p113L;
	}
      else if (v.ieee.exponent >= 0x7fff - LDBL_MANT_DIG)
	{
	  v.ieee.exponent -= LDBL_MANT_DIG;
	  if (u.ieee.exponent)
	    u.ieee.exponent += LDBL_MANT_DIG;
	  else
	    u.d *= 0x1p113L;
	}
      else /* if (u.ieee.exponent + v.ieee.exponent
		  <= IEEE854_LONG_DOUBLE_BIAS + LDBL_MANT_DIG) */
	{
	  if (u.ieee.exponent > v.ieee.exponent)
	    u.ieee.exponent += 2 * LDBL_MANT_DIG;
	  else
	    v.ieee.exponent += 2 * LDBL_MANT_DIG;
	  if (w.ieee.exponent <= 4 * LDBL_MANT_DIG + 4)
	    {
	      if (w.ieee.exponent)
		w.ieee.exponent += 2 * LDBL_MANT_DIG;
	      else
		w.d *= 0x1p226L;
	      adjust = -1;
	    }
	  /* Otherwise x * y should just affect inexact
	     and nothing else.  */
	}
      x = u.d;
      y = v.d;
      z = w.d;
    }
  /* Multiplication m1 + m2 = x * y using Dekker's algorithm.  */
#define C ((1LL << (LDBL_MANT_DIG + 1) / 2) + 1)
  long double x1 = x * C;
  long double y1 = y * C;
  long double m1 = x * y;
  x1 = (x - x1) + x1;
  y1 = (y - y1) + y1;
  long double x2 = x - x1;
  long double y2 = y - y1;
  long double m2 = (((x1 * y1 - m1) + x1 * y2) + x2 * y1) + x2 * y2;

  /* Addition a1 + a2 = z + m1 using Knuth's algorithm.  */
  long double a1 = z + m1;
  long double t1 = a1 - z;
  long double t2 = a1 - t1;
  t1 = m1 - t1;
  t2 = z - t2;
  long double a2 = t1 + t2;

  fenv_t env;
  feholdexcept (&env);
  fesetround (FE_TOWARDZERO);
  /* Perform m2 + a2 addition with round to odd.  */
  u.d = a2 + m2;

  if (__builtin_expect (adjust == 0, 1))
    {
      if ((u.ieee.mantissa3 & 1) == 0 && u.ieee.exponent != 0x7fff)
	u.ieee.mantissa3 |= fetestexcept (FE_INEXACT) != 0;
      feupdateenv (&env);
      /* Result is a1 + u.d.  */
      return a1 + u.d;
    }
  else if (__builtin_expect (adjust > 0, 1))
    {
      if ((u.ieee.mantissa3 & 1) == 0 && u.ieee.exponent != 0x7fff)
	u.ieee.mantissa3 |= fetestexcept (FE_INEXACT) != 0;
      feupdateenv (&env);
      /* Result is a1 + u.d, scaled up.  */
      return (a1 + u.d) * 0x1p113L;
    }
  else
    {
      if ((u.ieee.mantissa3 & 1) == 0)
	u.ieee.mantissa3 |= fetestexcept (FE_INEXACT) != 0;
      v.d = a1 + u.d;
      /* Ensure the addition is not scheduled after fetestexcept call.  */
      asm volatile ("" : : "m" (v));
      int j = fetestexcept (FE_INEXACT) != 0;
      feupdateenv (&env);
      /* Ensure the following computations are performed in default rounding
	 mode instead of just reusing the round to zero computation.  */
      asm volatile ("" : "=m" (u) : "m" (u));
      /* If a1 + u.d is exact, the only rounding happens during
	 scaling down.  */
      if (j == 0)
	return v.d * 0x1p-226L;
      /* If result rounded to zero is not subnormal, no double
	 rounding will occur.  */
      if (v.ieee.exponent > 226)
	return (a1 + u.d) * 0x1p-226L;
      /* If v.d * 0x1p-226L with round to zero is a subnormal above
	 or equal to LDBL_MIN / 2, then v.d * 0x1p-226L shifts mantissa
	 down just by 1 bit, which means v.ieee.mantissa3 |= j would
	 change the round bit, not sticky or guard bit.
	 v.d * 0x1p-226L never normalizes by shifting up,
	 so round bit plus sticky bit should be already enough
	 for proper rounding.  */
      if (v.ieee.exponent == 226)
	{
	  /* v.ieee.mantissa3 & 2 is LSB bit of the result before rounding,
	     v.ieee.mantissa3 & 1 is the round bit and j is our sticky
	     bit.  In round-to-nearest 001 rounds down like 00,
	     011 rounds up, even though 01 rounds down (thus we need
	     to adjust), 101 rounds down like 10 and 111 rounds up
	     like 11.  */
	  if ((v.ieee.mantissa3 & 3) == 1)
	    {
	      v.d *= 0x1p-226L;
	      if (v.ieee.negative)
		return v.d - 0x1p-16494L /* __LDBL_DENORM_MIN__ */;
	      else
		return v.d + 0x1p-16494L /* __LDBL_DENORM_MIN__ */;
	    }
	  else
	    return v.d * 0x1p-226L;
	}
      v.ieee.mantissa3 |= j;
      return v.d * 0x1p-226L;
    }
}
weak_alias (__fmal, fmal)
