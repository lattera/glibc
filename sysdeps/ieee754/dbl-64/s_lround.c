/* Round double value to long int.
   Copyright (C) 1997-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <fenv.h>
#include <limits.h>
#include <math.h>

#include <math_private.h>
#include <libm-alias-double.h>
#include <fix-fp-int-convert-overflow.h>


long int
__lround (double x)
{
  int32_t j0;
  uint32_t i1, i0;
  long int result;
  int sign;

  EXTRACT_WORDS (i0, i1, x);
  j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;
  sign = (i0 & 0x80000000) != 0 ? -1 : 1;
  i0 &= 0xfffff;
  i0 |= 0x100000;

  if (j0 < 20)
    {
      if (j0 < 0)
	return j0 < -1 ? 0 : sign;
      else
	{
	  i0 += 0x80000 >> j0;

	  result = i0 >> (20 - j0);
	}
    }
  else if (j0 < (int32_t) (8 * sizeof (long int)) - 1)
    {
      if (j0 >= 52)
	result = ((long int) i0 << (j0 - 20)) | ((long int) i1 << (j0 - 52));
      else
	{
	  uint32_t j = i1 + (0x80000000 >> (j0 - 20));
	  if (j < i1)
	    ++i0;

	  if (j0 == 20)
	    result = (long int) i0;
	  else
	    {
	      result = ((long int) i0 << (j0 - 20)) | (j >> (52 - j0));
#ifdef FE_INVALID
	      if (sizeof (long int) == 4
		  && sign == 1
		  && result == LONG_MIN)
		/* Rounding brought the value out of range.  */
		feraiseexcept (FE_INVALID);
#endif
	    }
	}
    }
  else
    {
      /* The number is too large.  Unless it rounds to LONG_MIN,
	 FE_INVALID must be raised and the return value is
	 unspecified.  */
#ifdef FE_INVALID
      if (FIX_DBL_LONG_CONVERT_OVERFLOW
	  && !(sign == -1
	       && (sizeof (long int) == 4
		   ? x > (double) LONG_MIN - 0.5
		   : x >= (double) LONG_MIN)))
	{
	  feraiseexcept (FE_INVALID);
	  return sign == 1 ? LONG_MAX : LONG_MIN;
	}
      else if (!FIX_DBL_LONG_CONVERT_OVERFLOW
	       && sizeof (long int) == 4
	       && x <= (double) LONG_MIN - 0.5)
	{
	  /* If truncation produces LONG_MIN, the cast will not raise
	     the exception, but may raise "inexact".  */
	  feraiseexcept (FE_INVALID);
	  return LONG_MIN;
	}
#endif
      return (long int) x;
    }

  return sign * result;
}

libm_alias_double (__lround, lround)
