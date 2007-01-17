/* Round to int long double floating-point values.
   IBM extended format long double version.
   Copyright (C) 2006, 2007 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* This has been coded in assembler because GCC makes such a mess of it
   when it's coded in C.  */

#include <math.h>
#include <math_ldbl_opt.h>
#include <float.h>
#include <ieee754.h>


#ifdef __STDC__
long double
__roundl (long double x)
#else
long double
__roundl (x)
     long double x;
#endif
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

      /* Adjust the result when the remainder is exactly 0.5.  nearbyint
	 rounds values halfway between integers to the nearest even
	 integer.  roundl must round away from zero.
	 Also correct cases where nearbyint returns an incorrect value
	 for LO.  */
      xh -= lo;
      ldbl_canonicalize (&xh, &xl);
      if (xh == 0.5)
	{
	  if (xl > 0.0 || (xl == 0.0 && orig_xh > 0.0))
	    lo += 1.0;
	}
      else if (-xh == 0.5)
	{
	  if (xl < 0.0 || (xl == 0.0 && orig_xh < 0.0))
	    lo -= 1.0;
	}

      /* Ensure the final value is canonical.  In certain cases,
	 rounding causes hi,lo calculated so far to be non-canonical.  */
      xh = hi;
      xl = lo;
      ldbl_canonicalize (&xh, &xl);
    }

  return ldbl_pack (xh, xl);
}

long_double_symbol (libm, __roundl, roundl);
