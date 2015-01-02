/* Copyright (C) 2005-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@redhat.com>, 2005.

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

#include <stdbool.h>
#include <math.h>
#include <complex.h>

attribute_hidden
long double _Complex
__multc3 (long double a, long double b, long double c, long double d)
{
  long double ac, bd, ad, bc, x, y;

  ac = a * c;
  bd = b * d;
  ad = a * d;
  bc = b * c;

  x = ac - bd;
  y = ad + bc;

  if (isnan (x) && isnan (y))
    {
      /* Recover infinities that computed as NaN + iNaN.  */
      bool recalc = 0;
      if (__isinf_nsl (a) || __isinf_nsl (b))
	{
	  /* z is infinite.  "Box" the infinity and change NaNs in
	     the other factor to 0.  */
	  a = __copysignl (__isinf_nsl (a) ? 1 : 0, a);
	  b = __copysignl (__isinf_nsl (b) ? 1 : 0, b);
	  if (isnan (c)) c = __copysignl (0, c);
	  if (isnan (d)) d = __copysignl (0, d);
	  recalc = 1;
	}
     if (__isinf_nsl (c) || __isinf_nsl (d))
	{
	  /* w is infinite.  "Box" the infinity and change NaNs in
	     the other factor to 0.  */
	  c = __copysignl (__isinf_nsl (c) ? 1 : 0, c);
	  d = __copysignl (__isinf_nsl (d) ? 1 : 0, d);
	  if (isnan (a)) a = __copysignl (0, a);
	  if (isnan (b)) b = __copysignl (0, b);
	  recalc = 1;
	}
     if (!recalc
	  && (__isinf_nsl (ac) || __isinf_nsl (bd)
	      || __isinf_nsl (ad) || __isinf_nsl (bc)))
	{
	  /* Recover infinities from overflow by changing NaNs to 0.  */
	  if (isnan (a)) a = __copysignl (0, a);
	  if (isnan (b)) b = __copysignl (0, b);
	  if (isnan (c)) c = __copysignl (0, c);
	  if (isnan (d)) d = __copysignl (0, d);
	  recalc = 1;
	}
      if (recalc)
	{
	  x = INFINITY * (a * c - b * d);
	  y = INFINITY * (a * d + b * c);
	}
    }

  return x + I * y;
}
