/* Round argument to nearest integral value according to current rounding
   direction.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Schwab <schwab@issan.informatik.uni-dortmund.de>

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

#define __LIBC_M81_MATH_INLINES
#include <math.h>
#include "math_private.h"

long long int
__rinttoll (long double x)
{
  int32_t se, sx;
  u_int32_t h, l;
  long long int result;

  x = __m81_u(__rintl) (x);

  /* We could use __fixxfdi from libgcc, but here we can take advantage of
     the known floating point format.  */
  GET_LDOUBLE_WORDS (se, h, l, x);

  sx = se & (1 << 15);
  se = (se ^ sx) - 0x3fff;

  if (se < 64)
    {
      if (se > 31)
	result = (((long long int) (h >> (63 - se)) << 32)
		  | (l >> (63 - se)) | (h << (se - 31)));
      else
	result = h >> (31 - se);
      if (sx)
	result = -result;
    }
  else
    /* Too large.  The number is either +-inf or NaN or it is too
       large to be effected by rounding.  The standard leaves it
       undefined what to return when the number is too large to fit in
       a `long long int'.  */
    result = -1LL;

  return result;
}

weak_alias (__rinttoll, rinttoll)
