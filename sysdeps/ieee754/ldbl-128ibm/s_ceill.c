/* Ceil (round to +inf) long double floating-point values.
   IBM extended format long double version.
   Copyright (C) 2006-2015 Free Software Foundation, Inc.
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
__ceill (long double x)
{
  double xh, xl, hi, lo;

  ldbl_unpack (x, &xh, &xl);

  /* Return Inf, Nan, +/-0 unchanged.  */
  if (__builtin_expect (xh != 0.0
			&& __builtin_isless (__builtin_fabs (xh),
					     __builtin_inf ()), 1))
    {
      double orig_xh;

      /* Long double arithmetic, including the canonicalisation below,
	 only works in round-to-nearest mode.  */

      /* Convert the high double to integer.  */
      orig_xh = xh;
      hi = ldbl_nearbyint (xh);

      /* Subtract integral high part from the value.  */
      xh -= hi;
      ldbl_canonicalize (&xh, &xl);

      /* Now convert the low double, adjusted for any remainder from the
         high double.  */
      lo = ldbl_nearbyint (xh);

      /* Adjust the result when the remainder is non-zero.  nearbyint
         rounds values to the nearest integer, and values halfway
         between integers to the nearest even integer.  ceill must
         round towards +Inf.  */
      xh -= lo;
      ldbl_canonicalize (&xh, &xl);

      if (xh > 0.0 || (xh == 0.0 && xl > 0.0))
	lo += 1.0;

      /* Ensure the final value is canonical.  In certain cases,
         rounding causes hi,lo calculated so far to be non-canonical.  */
      xh = hi;
      xl = lo;
      ldbl_canonicalize (&xh, &xl);

      /* Ensure we return -0 rather than +0 when appropriate.  */
      if (orig_xh < 0.0)
	xh = -__builtin_fabs (xh);
    }

  return ldbl_pack (xh, xl);
}

long_double_symbol (libm, __ceill, ceill);
