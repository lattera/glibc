/* Round argument to nearest integral value according to current rounding
   direction.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <math.h>

#include "math_private.h"

#ifdef NO_LONG_DOUBLE
/* The `long double' is in fact the IEEE `double' type.  */
static long double two52[2] =
{
  4.50359962737049600000e+15, /* 0x43300000, 0x00000000 */
 -4.50359962737049600000e+15, /* 0xC3300000, 0x00000000 */
};


long int
__lrint (long double x)
{
  int32_t j0,sx;
  u_int32_t i0,i1,i;
  long double t, w;
  long int result;

  EXTRACT_WORDS (i0, i1, x);

  sx = i0 >> 31;
  j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;

  if (j0 < 20)
    {
      if (j0 < 0)
	{
	  if (((i0 & 0x7fffffff) | i1) == 0)
	    /* The number is 0.  */
	    result = 0;
	  else
	    {
	      i1 |= i0;
	      i0 &= 0xfffe0000;
	      i0 |= ((i1 | -i1) >> 12) & 0x80000;
	      SET_HIGH_WORD (x, i0);
	      w = two52[sx] + x;
	      t = w - two52[sx];
	      GET_HIGH_WORD (i0, t);
	      if ((i0 & 0x7fffffff) >= 0x3fff0000)
		result = sx ? -1 : 1;
	      else
		result = 0;
	    }
	}
      else
	{
	  u_int32_t i = 0x000fffff >> j0;
	  if (((i0 & i) | i1) == 0)
	    {
	      /* X is not integral.  */
	      i >>= 1;
	      if (((i0 & i) | i1) != 0)
		{
		  if (j0 == 19)
		    i1 = 0x40000000;
		  else
		    i0 = (i0 & (~i)) | (0x20000 >> j0);

		  INSERT_WORDS (x, i0, i1);
		  w = two52[sx] + x;
		  x = w - two52[sx];
		  EXTRACT_WORDS (i0, i1, x);

		  j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;
		}
	    }

	  result = ((i0 >> (20 - j0)) & 0xfffff) | (0x00100000 >> (20 - j0));
	  if (sx)
	    result = -result;
	}
    }
  else if ((unsigned int) j0 < sizeof (long int) * 8 && j0 < 53)
    {
      i = ((u_int32_t) (0xffffffff)) >> (j0 - 20);
      if ((i1 & i) != 0)
	{
	  /* x is not integral.  */
	  i >>= 1;
	  if ((i1 & i) != 0)
	    i1 = (i1 & (~i)) | (0x40000000 >> (j0 - 20));
	}

      INSERT_WORDS (x, i0, i1);
      w = two52[sx] + x;
      x = w - two52[sx];
      EXTRACT_WORDS (i0, i1, x);

      j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;

      result = i0 | 0x00100000;
      if (j0 > 20)
	{
	  result <<= j0 - 20;
	  result |= i1 >> (52 - j0);
	}
      if (sx)
	result = -result;
    }
  else
    /* Too large.  The number is either +-inf or NaN or it is too
       large to be effected by rounding.  The standard leaves it
       undefined what to return when the number is too large to fit in
       a `long int'.  */
    result = (long int) x;

  return result;
}

#else
static long double two63[2] =
{
  9.223372036854775808000000e+18, /* 0x403E, 0x00000000, 0x00000000 */
 -9.223372036854775808000000e+18  /* 0xC03E, 0x00000000, 0x00000000 */
};


long int
__lrint (long double x)
{
  int32_t se,j0,sx;
  u_int32_t i0,i1,i;
  long int result;
  long double w, t;

  GET_LDOUBLE_WORDS (se, i0, i1, x);

  sx = (se >> 15) & 1;
  j0 = (se & 0x7fff) - 0x3fff;

  if (j0 < 31)
    {
      if (j0 < 0)
	{
	  if (((se & 0x7fff) | i0 | i1) == 0)
	    /* The number is 0.  */
	    result = 0;
	  else
	    {
	      i1 |= i0;
	      i0 &= 0xe0000000;
	      i0 |= (i1 | -i1) & 0x80000000;
	      SET_LDOUBLE_MSW (x, i0);
	      w = two63[sx] + x;
	      t = w - two63[sx];
	      GET_LDOUBLE_EXP (i0, t);
	      if ((i0 & 0x7fff) >= 0x3fff)
		result = sx ? -1 : 1;
	      else
		result = 0;
	    }
	}
      else
	{
	  u_int32_t i = 0x7fffffff >> j0;
	  if (((i0 & i) | i1) == 0)
	    {
	      /* X is not integral.  */
	      i >>= 1;
	      if (((i0 & i) | i1) != 0)
		{
		  if (j0 == 31)
		    i1 = 0x40000000;
		  else
		    i0 = (i0 & (~i)) | (0x20000000 >> j0);

		  SET_LDOUBLE_WORDS (x, se, i0, i1);
		  w = two63[sx] + x;
		  x = w - two63[sx];
		  GET_LDOUBLE_WORDS (se, i0, i1, x);

		  sx = (se >> 15) & 1;
		  j0 = (se & 0x7fff) - 0x3fff;
		}
	    }


	  result = i0 >> (31 - j0);
	}
    }
  else if ((unsigned int) j0 < sizeof (long int) * 8 && j0 < 64)
    {
      i = ((u_int32_t) (0xffffffff)) >> (j0 - 31);
      if ((i1 & i) != 0)
	{
	  /* x is not integral.  */
	  i >>= 1;
	  if ((i1 & i) != 0)
	    i1 = (i1 & (~i)) | (0x40000000 >> (j0 - 31));
	}

      SET_LDOUBLE_WORDS (x, se, i0, i1);
      w = two63[sx] + x;
      x = w - two63[sx];
      GET_LDOUBLE_WORDS (se, i0, i1, x);

      j0 = (se & 0x7fff) - 0x3fff;

      result = i0;
      if (j0 > 31)
	{
	  result <<= j0 - 31;
	  result |= i1 >> (63 - j0);
	}
    }
  else
    /* Too large.  The number is either +-inf or NaN or it is too
       large to be effected by rounding.  The standard leaves it
       undefined what to return when the number is too large to fit in
       a `long int'.  */
    result = (long int) x;

  return result;
}
#endif

weak_alias (__lrint, lrint)
