/* Copyright (C) 1991 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <errno.h>
#include <math.h>

/* Return the inverse sine of X.  */
double
DEFUN(asin, (x), double x)
{
  double abs_x;

  if (__isnan(x))
    {
      errno = EDOM;
      return x;
    }

  abs_x = fabs(x);

  if (abs_x <= 0.5)
    return atan2(x, sqrt(1.0 - (x * x)));
  else
    {
      double t = 1.0 - abs_x;
      return atan2(x, sqrt((t + t) - (t * t)));
    }
}
