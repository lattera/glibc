/* Miscellaneous tests which don't fit anywhere else.
   Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
#include <stdio.h>


int
main (void)
{
  int result = 0;

#ifndef NO_LONG_DOUBLE
  {
    long double x = 0x100000001ll + (long double) 0.5;
    long double q;
    long double r;

    r = modfl (x, &q);
    if (q != (long double) 0x100000001ll || r != 0.5)
      {
	printf ("modfl (%Lg, ...) failed\n", x);
	result = 1;
      }
  }
#endif

  {
    double x = 0x100000001ll + (double) 0.5;
    double q;
    double r;

    r = modf (x, &q);
    if (q != (double) 0x100000001ll || r != 0.5)
      {
	printf ("modf (%g, ...) failed\n", x);
	result = 1;
      }
  }

  return result;
}
