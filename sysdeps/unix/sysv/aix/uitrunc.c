/* Copyright (C) 2000 Free Software Foundation, Inc.
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

/* The uitrunc function returns the nearest unsigned integer
   to the x parameter in the direction of 0. This actions is
   equivalent to truncation off the fraction bits of the x
   parameter and then converting x to an unsigned integer. */
unsigned int
__uitrunc (double x)
{
  double xrf;
  unsigned int xr;
  xr = (unsigned int) x;
  xrf = (double) xr;
  if (x >= 0.0)
    if (x - xrf >= 0.5 && x - xrf < 1.0 && x + 1 > 0)
      return x + 1;
    else
      return x;
  else
    if (xrf - x >= 0.5 && xrf - x < 1.0 && x - 1 < 0)
      return x - 1;
    else
      return x;
}
