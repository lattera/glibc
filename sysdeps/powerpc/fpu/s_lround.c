/* Round double value to long int.
   Copyright (C) 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <math.h>

/* I think that what this routine is supposed to do is round a value
   to the nearest integer, with values exactly on the boundary rounded
   away from zero.  */
/* This routine relies on (long int)x, when x is out of range of a long int,
   clipping to MAX_LONG or MIN_LONG.  */

long int
__lround (double x)
{
  double xrf;
  long int xr;
  xr = (long int) x;
  xrf = (double) xr;
  if (x >= 0.0)
    if (x - xrf >= 0.5 && x - xrf < 1.0 && x+1 > 0)
      return x+1;
    else
      return x;
  else
    if (xrf - x >= 0.5 && xrf - x < 1.0 && x-1 < 0)
      return x-1;
    else
      return x;
}
weak_alias (__lround, lround)
#ifdef NO_LONG_DOUBLE
strong_alias (__lround, __lroundl)
weak_alias (__lround, lroundl)
#endif
