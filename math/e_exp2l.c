/* Compute 2^x.
   Copyright (C) 2012-2016 Free Software Foundation, Inc.
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

#include <math.h>
#include <math_private.h>
#include <float.h>

/* To avoid spurious underflows, use this definition to treat IBM long
   double as approximating an IEEE-style format.  */
#if LDBL_MANT_DIG == 106
# undef LDBL_EPSILON
# define LDBL_EPSILON 0x1p-106L
#endif

long double
__ieee754_exp2l (long double x)
{
  if (__glibc_likely (isless (x, (long double) LDBL_MAX_EXP)))
    {
      if (__builtin_expect (isgreaterequal (x, (long double) (LDBL_MIN_EXP
							      - LDBL_MANT_DIG
							      - 1)), 1))
	{
	  int intx = (int) x;
	  long double fractx = x - intx;
	  long double result;
	  if (fabsl (fractx) < LDBL_EPSILON / 4.0L)
	    result = __scalbnl (1.0L + fractx, intx);
	  else
	    result = __scalbnl (__ieee754_expl (M_LN2l * fractx), intx);
	  math_check_force_underflow_nonneg (result);
	  return result;
	}
      else
	{
	  /* Underflow or exact zero.  */
	  if (isinf (x))
	    return 0;
	  else
	    return LDBL_MIN * LDBL_MIN;
	}
    }
  else
    /* Infinity, NaN or overflow.  */
    return LDBL_MAX * x;
}
strong_alias (__ieee754_exp2l, __exp2l_finite)
