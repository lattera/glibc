/* Compute remainder and a congruent to the quotient.
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


static const double zero = 0.0;


double
__remquo (double x, double y, int *quo)
{
  int32_t hx,hp;
  u_int32_t sx,lx,lp;
  int cquo;

  EXTRACT_WORDS (hx, lx, x);
  EXTRACT_WORDS (hp, lp, p);
  sx = hx & 0x80000000;
  qs = (sx ^ (hp & 0x80000000)) >> 31;
  hp &= 0x7fffffff;
  hx &= 0x7fffffff;

  /* Purge off exception values.  */
  if ((hp | lp) == 0)
    return (x * p) / (x * p); 			/* p = 0 */
  if ((hx >= 0x7ff00000)			/* x not finite */
      || ((hp >= 0x7ff00000)			/* p is NaN */
	  && (((hp - 0x7ff00000) | lp) != 0)))
    return (x * p) / (x * p);

  if (hp <= 0x7fbfffff)
    {
      x = __ieee754_fmod (x, 8 * p);		/* now x < 8p */

      if (fabs (x) >= 4 * fabs (p))
	cquo += 4;
    }

  if (((hx - hp) | (lx - lp)) == 0)
    {
      *quo = qs ? -1 : 1;
      return zero * x;
    }

  x  = fabs (x);
  p  = fabs (p);
  cquo = 0;

  if (x >= 2 * p)
    {
      x -= 4 * p;
      cquo += 2;
    }
  if (x >= p)
    {
      x -= 2 * p;
      ++cquo;
    }

  if (hp < 0x00200000)
    {
      if (x + x > p)
	{
	  x -= p;
	  if (x + x >= p)
	    x -= p;
	}
    }
  else
    {
      double p_half = 0.5 * p;
      if(x > p_half)
	{
	  x -= p;
	  if (x >= p_half)
	    x -= p;
	}
    }

  *quo = qs ? -cquo : cquo;

  GET_HIGH_WORD (hx, x);
  SET_HIGH_WORD (x, hx ^ sx);
  return x;
}
weak_alias (__remquo, remquo)
