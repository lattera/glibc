/* Copyright (C) 1992, 1995 Free Software Foundation, Inc.
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

/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <ansidecl.h>
#include <math.h>
#include <float.h>
#include "ieee754.h"

/* Return the remainder of X/Y.  */
double
DEFUN(__drem, (x, y),
      double x AND double y)
{
  union ieee754_double ux, uy;

  ux.d = x;
  uy.d = y;
#define x ux.d
#define	y uy.d

  uy.ieee.negative = 0;

  if (!__finite (x) || y == 0.0)
    return NAN;
  else if (__isnan (y))
    return y;
  else if (__isinf (y))
    return x;
  else if (uy.ieee.exponent <= 1)
    {
      /* Subnormal (or almost subnormal) Y value.  */
      double b = __scalb (1.0, 54);
      y *= b;
      x = __drem (x, y);
      x *= b;
      return __drem (x, y) / b;
    }
  else if (y >= 1.7e308 / 2)
    {
      y /= 2;
      x /= 2;
      return __drem (x, y) * 2;
    }
  else
    {
      union ieee754_double a;
      double b;
      unsigned int negative = ux.ieee.negative;
      a.d = y + y;
      b = y / 2;
      ux.ieee.negative = 0;
      while (x > a.d)
	{
	  unsigned short int k = ux.ieee.exponent - a.ieee.exponent;
	  union ieee754_double tmp;
	  tmp.d = a.d;
	  tmp.ieee.exponent += k;
	  if (x < tmp.d)
	    --tmp.ieee.exponent;
	  x -= tmp.d;
	}
      if (x > b)
	{
	  x -= y;
	  if (x >= b)
	    x -= y;
	}
      ux.ieee.negative ^= negative;
      return x;
    }
}

weak_alias (__drem, drem)
