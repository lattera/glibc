/* Return quiet nan.
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

/* XXX The ISO C 9X standard mentions a `n-char-sequence' which is provided
   as the argument to this function but I have no clue what this means.
   Perhaps it is a description of the bits set in the mantissa.  */
#include <math.h>
#ifdef HANDLE_TAGP
# include <stdlib.h>
# include <string.h>
#else
# include <ieee754.h>
#endif


float
__nanf (const char *tagp)
{
#ifdef HANDLE_TAGP
  /* If we ever should have use of the TAGP parameter we will use the
     strtod function to analyze it.  */
  char buf[6 + strlen (tagp)];
  sprintf (buf, "NAN(%s)", tagp);
  return strtof (buf, NULL);
#else
  static const union ieee754_float nan_value =
  { ieee: { mantissa: 0x1, exponent: 0xff, negative: 0 } };
  return nan_value.f;
#endif
}
weak_alias (__nanf, nanf)
