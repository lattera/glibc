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

/* Return the inverse cosine of X.  */
double
DEFUN(acos, (x), double x)
{
  double t;

  if (__isnan(x))
    {
      errno = EDOM;
      return x;
    }

  if (x == -1.0)
    /* If X is -1, the general formula blows up (zero divided by zero loses),
       but we know that acos(-1) = pi.  */
    t = atan2(1.0, 0.0);
  else
    t = atan2(sqrt((1.0 - x) / (1.0 + x)), 1.0);
  return t + t;
}
