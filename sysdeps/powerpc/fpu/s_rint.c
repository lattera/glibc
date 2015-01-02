/* Round a 64-bit floating point value to the nearest integer.
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

double
__rint (double x)
{
  static const float TWO52 = 4503599627370496.0;

  if (fabs (x) < TWO52)
    {
      if (x > 0.0)
	{
	  x += TWO52;
	  x -= TWO52;
	}
      else if (x < 0.0)
	{
	  x = TWO52 - x;
	  x = -(x - TWO52);
	}
    }

  return x;
}
weak_alias (__rint, rint)
#ifdef NO_LONG_DOUBLE
strong_alias (__rint, __rintl)
weak_alias (__rint, rintl)
#endif
