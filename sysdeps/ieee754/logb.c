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

#include <ansidecl.h>
#include <math.h>
#include <float.h>
#include "ieee754.h"

/* Return the base 2 signed integral exponent of X.  */
double
DEFUN(__logb, (x), double x)
{
  union ieee754_double u;

  if (__isnan (x))
    return x;
  else if (__isinf (x))
    return HUGE_VAL;
  else if (x == 0.0)
    return - HUGE_VAL;

  u.d = x;

  if (u.ieee.exponent == 0)
    /* A denormalized number.
       Multiplying by 2 ** DBL_MANT_DIG normalizes it;
       we then subtract the DBL_MANT_DIG we added to the exponent.  */
    return (__logb (x * ldexp (1.0, DBL_MANT_DIG)) - DBL_MANT_DIG);

  return (int) u.ieee.exponent - (DBL_MAX_EXP - 1);
}

weak_alias (__logb, logb)
