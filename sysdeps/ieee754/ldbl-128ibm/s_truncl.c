/* Truncate (toward zero) long double floating-point values.
   IBM extended format long double version.
   Copyright (C) 2006-2018 Free Software Foundation, Inc.
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
#include <math_ldbl_opt.h>
#include <float.h>
#include <ieee754.h>


long double
__truncl (long double x)
{
  double xh, xl, hi, lo;

  ldbl_unpack (x, &xh, &xl);

  /* Return Inf, Nan, +/-0 unchanged.  */
  if (__builtin_expect (xh != 0.0
			&& __builtin_isless (__builtin_fabs (xh),
					     __builtin_inf ()), 1))
    {
      hi = __trunc (xh);
      if (hi != xh)
	{
	  /* The high part is not an integer; the low part does not
	     affect the result.  */
	  xh = hi;
	  xl = 0;
	}
      else
	{
	  /* The high part is a nonzero integer.  */
	  lo = xh > 0 ? __floor (xl) : __ceil (xl);
	  xh = hi;
	  xl = lo;
	  ldbl_canonicalize_int (&xh, &xl);
	}
    }
  else
    /* Quiet signaling NaN arguments.  */
    xh += xh;

  return ldbl_pack (xh, xl);
}

long_double_symbol (libm, __truncl, truncl);
