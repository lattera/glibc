/* Round float value to long long int.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <math.h>

/* I think that what this routine is supposed to do is round a value
   to the nearest integer, with values exactly on the boundary rounded
   away from zero.  */
/* This routine relies on (long long)x, when x is out of range of a long long,
   clipping to MAX_LLONG or MIN_LLONG.  */

long long int
__llroundf (float x)
{
  float xrf;
  long long int xr;
  xr = (long long int) x;
  xrf = (float) xr;
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
weak_alias (__llroundf, llroundf)
