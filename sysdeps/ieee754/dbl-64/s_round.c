/* Round double to integer away from zero.
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

#include <math.h>

#include <math_private.h>
#include <libm-alias-double.h>


double
__round (double x)
{
  int32_t i0, j0;
  uint32_t i1;

  EXTRACT_WORDS (i0, i1, x);
  j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;
  if (j0 < 20)
    {
      if (j0 < 0)
	{
	  i0 &= 0x80000000;
	  if (j0 == -1)
	    i0 |= 0x3ff00000;
	  i1 = 0;
	}
      else
	{
	  uint32_t i = 0x000fffff >> j0;
	  if (((i0 & i) | i1) == 0)
	    /* X is integral.  */
	    return x;

	  i0 += 0x00080000 >> j0;
	  i0 &= ~i;
	  i1 = 0;
	}
    }
  else if (j0 > 51)
    {
      if (j0 == 0x400)
	/* Inf or NaN.  */
	return x + x;
      else
	return x;
    }
  else
    {
      uint32_t i = 0xffffffff >> (j0 - 20);
      if ((i1 & i) == 0)
	/* X is integral.  */
	return x;

      uint32_t j = i1 + (1 << (51 - j0));
      if (j < i1)
	i0 += 1;
      i1 = j;
      i1 &= ~i;
    }

  INSERT_WORDS (x, i0, i1);
  return x;
}
libm_alias_double (__round, round)
