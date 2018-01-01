/* Round double value to long long int.
   Copyright (C) 1997-2018 Free Software Foundation, Inc.
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

#include <limits.h>
#include <math.h>
#include <math_ldbl_opt.h>
#include <math_private.h>
#include <stdint.h>
#include <libm-alias-double.h>

/* Round to the nearest integer, with values exactly on a 0.5 boundary
   rounded away from zero, regardless of the current rounding mode.
   If (long long)x, when x is out of range of a long long, clips at
   LLONG_MAX or LLONG_MIN, then this implementation also clips.  */

long long int
__llround (double x)
{
  long long xr;
  if (HAVE_PPC_FCTIDZ)
    xr = (long long) x;
  else
    {
      /* Avoid incorrect exceptions from libgcc conversions (as of GCC
	 5): <https://gcc.gnu.org/bugzilla/show_bug.cgi?id=59412>.  */
      if (fabs (x) < 0x1p31)
	xr = (long long int) (long int) x;
      else
	{
	  uint64_t i0;
	  EXTRACT_WORDS64 (i0, x);
	  int exponent = ((i0 >> 52) & 0x7ff) - 0x3ff;
	  if (exponent < 63)
	    {
	      unsigned long long int mant
		= (i0 & ((1ULL << 52) - 1)) | (1ULL << 52);
	      if (exponent < 52)
		/* llround is not required to raise "inexact".  */
		mant >>= 52 - exponent;
	      else
		mant <<= exponent - 52;
	      xr = (long long int) ((i0 & (1ULL << 63)) != 0 ? -mant : mant);
	    }
	  else if (x == (double) LLONG_MIN)
	    xr = LLONG_MIN;
	  else
	    xr = (long long int) (long int) x << 32;
	}
    }
  /* Avoid spurious "inexact" converting LLONG_MAX to double, and from
     subtraction when the result is out of range, by returning early
     for arguments large enough that no rounding is needed.  */
  if (!(fabs (x) < 0x1p52))
    return xr;
  double xrf = (double) xr;

  if (x >= 0.0)
    {
      if (x - xrf >= 0.5)
	xr += (long long) ((unsigned long long) xr + 1) > 0;
    }
  else
    {
      if (xrf - x >= 0.5)
	xr -= (long long) ((unsigned long long) xr - 1) < 0;
    }
  return xr;
}
libm_alias_double (__llround, llround)
