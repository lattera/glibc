/* Copyright (C) 1991, 1992, 1994 Free Software Foundation, Inc.
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
#include <math.h>

#ifdef	__GNUC__

__CONSTVALUE double
DEFUN(atan2, (y, x), double y AND double x)
{
  static CONST double one = 1.0, zero = 0.0;
  double signx, signy;
  double pi, PIo4, PIo2;

  if (__isnan(x))
    return x;
  if (__isnan(y))
    return y;

  signy = __copysign(one, y);
  signx = __copysign(one, x);

  asm("fmovecr%.x %1, %0" : "=f" (pi) : "i" (0));
  PIo2 = pi / 2;
  PIo4 = pi / 4;

  if (y == zero)
    return signx == one ? y : __copysign(pi, signy);

  if (x == zero)
    return __copysign(PIo2, signy);

  if (__isinf(x))
    {
      if (__isinf(y))
	return __copysign(signx == one ? PIo4 : 3 * PIo4, signy);
      else
	return __copysign(signx == one ? zero : pi, signy);
    }

  if (__isinf(y))
    return __copysign(PIo2, signy);

  y = fabs(y);

  if (x < 0.0)
    /* X is negative.  */
    return __copysign(pi - atan(y / -x), signy);

  return __copysign(atan(y / x), signy);
}

#else
#include <sysdeps/generic/atan2.c>
#endif
