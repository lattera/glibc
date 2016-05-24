/* Round double to integer away from zero.
   Copyright (C) 1997-2016 Free Software Foundation, Inc.
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

#include <math.h>

#include <math_private.h>
#include <stdint.h>


double
__round (double x)
{
  int64_t i0, j0;

  EXTRACT_WORDS64 (i0, x);
  j0 = ((i0 >> 52) & 0x7ff) - 0x3ff;
  if (__glibc_likely (j0 < 52))
    {
      if (j0 < 0)
	{
	  i0 &= UINT64_C(0x8000000000000000);
	  if (j0 == -1)
	    i0 |= UINT64_C(0x3ff0000000000000);
	}
      else
	{
	  uint64_t i = UINT64_C(0x000fffffffffffff) >> j0;
	  if ((i0 & i) == 0)
	    /* X is integral.  */
	    return x;

	  i0 += UINT64_C(0x0008000000000000) >> j0;
	  i0 &= ~i;
	}
    }
  else
    {
      if (j0 == 0x400)
	/* Inf or NaN.  */
	return x + x;
      else
	return x;
    }

  INSERT_WORDS64 (x, i0);
  return x;
}
weak_alias (__round, round)
#ifdef NO_LONG_DOUBLE
strong_alias (__round, __roundl)
weak_alias (__round, roundl)
#endif
