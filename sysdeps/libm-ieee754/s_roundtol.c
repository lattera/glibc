/* Round long double value to long int.
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

long int
__roundtol (long double x)
{
  int32_t j0;
  u_int32_t i1, i0;
  long int result;

  EXTRACT_WORDS (i0, i1, x);
  j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;
  if (j0 < 20)
    {
      if (j0 < 0)
	result = j0 < -1 ? 0 : ((i0 & 0x80000000) ? -1 : 1);
      else
	{
	  u_int32_t i = 0xfffff >> j0;
	  if (((i0 & i) | i1) == 0)
	    result = (long int) ((i0 & 0xfffff) | 0x100000) >> j0;
	  else
	    {
	      /* X is not integral.  */
	      u_int32_t j = i0 + (0x80000 >> j0);
              if (j < i0)
		result = (long int) 0x80000 >> (20 - j0);
	      else
		result = (j | 0x100000) >> (20 - j0);
	    }
	}
    }
  else if (j0 >= 8 * sizeof (long int) || j0 > 51)
    {
      /* The number is too large.  It is left implementation defined
	 what happens.  */
      result = (long int) x;
    }
  else
    {
      i = ((u_int32_t) (0xffffffff)) >> (j0 - 20);
      if ((i1 & i) != 0)
	{
	  /* x is not integral.  */
	  u_int32_t j = i1 + (0x80000000 >> (j0 - 20));
	  if (j < i1)
	    {
	      j = i0 + 1;
	      if ((j & 0xfffff) == 0)
		{
		  if (sizeof (long int) <= 4)
		    /* Overflow.  */
		    result = (long int) x;
		  else
		    result = 1l << (j0 + 1);
		}
	      else
		result = (long int) ((i0 & 0xfffff) | 0x100000) << (j0 - 31);
	    }
	  else
	    {
	      result = (long int) ((i0 & 0xfffff) | 0x100000) << (j0 - 31);
	      if (sizeof (long int) > 4 && j0 > 31)
		result |= j >> (63 - j0);
	    }
	}
      else
	{
	  result = (long int) ((i0 & 0xfffff) | 0x100000) << (j0 - 31);
	  if (sizeof (long int) > 4 && j0 > 31)
	    result |= j >> (63 - j0);
	}
    }

  return i0 & 0x80000000 ? -result : result;
}
#else
long int
__roundtol (long double x)
{
  int32_t j0;
  u_int32_t se, i1, i0;
  long int result;

  GET_LDOUBLE_WORDS (se, i0, i1, x);
  j0 = (se & 0x7fff) - 0x3fff;
  if (j0 < 31)
    {
      if (j0 < 0)
	result = j0 < -1 ? 0 : 1;
      else
	{
	  u_int32_t i = 0x7fffffff >> j0;
	  if (((i0 & i) | i1) == 0)
	    result = (long int) i0 >> j0;
	  else
	    {
	      /* X is not integral.  */
	      u_int32_t j = i0 + (0x40000000 >> j0);
              if (j < i0)
		result = 0x80000000l >> (30 - j0);
	      else
		result = j >> (31 - j0);
	    }
	}
    }
  else if ((unsigned int) j0 >= 8 * sizeof (long int) || j0 > 62)
    {
      /* The number is too large.  It is left implementation defined
	 what happens.  */
      result = (long int) x;
    }
  else
    {
      u_int32_t i = ((u_int32_t) (0xffffffff)) >> (j0 - 31);
      if ((i1 & i) != 0)
	{
	  /* x is not integral.  */
	  u_int32_t j = i1 + (0x80000000 >> (j0 - 31));
	  if (j < i1)
	    {
	      j = i0 + 1;
	      if (j == 0)
		{
		  if (sizeof (long int) <= 4)
		    /* Overflow.  */
		    result = (long int) x;
		  else
		    result = 1l << (j0 + 1);
		}
	      else
		result = (long int) i0 << (j0 - 31);
	    }
	  else
	    {
	      result = (long int) i0 << (j0 - 31);
	      if (sizeof (long int) > 4 && j0 > 31)
		result |= j >> (63 - j0);
	    }
	}
      else
	{
	  result = (long int) i0 << (j0 - 31);
	  if (sizeof (long int) > 4 && j0 > 31)
	    result |= i1 >> (63 - j0);
	}
    }

  return se & 0x8000 ? -result : result;
}
#endif
weak_alias (__roundtol, roundtol)
