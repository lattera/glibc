/* Round double value to long long int.
   Copyright (C) 1997-2015 Free Software Foundation, Inc.
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
#include <math_ldbl_opt.h>

/* Round to the nearest integer, with values exactly on a 0.5 boundary
   rounded away from zero, regardless of the current rounding mode.
   If (long long)x, when x is out of range of a long long, clips at
   LLONG_MAX or LLONG_MIN, then this implementation also clips.  */

long long int
__llround (double x)
{
  long long xr = (long long) x;
  double xrf = (double) xr;

  if (x >= 0.0)
    {
      if (x - xrf >= 0.5)
	xr += (long long) ((unsigned long long) xr + 1) > 0;
    }
  else
    {
      if (xrf - x >= 0.5)
	xr -= (long long) ((unsigned long long) xr - 1) < 0;
    }
  return xr;
}
weak_alias (__llround, llround)
#ifdef NO_LONG_DOUBLE
strong_alias (__llround, __llroundl)
weak_alias (__llround, llroundl)
#endif
#if LONG_DOUBLE_COMPAT (libm, GLIBC_2_1)
compat_symbol (libm, __llround, llroundl, GLIBC_2_1);
#endif
