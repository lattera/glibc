/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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

#define __LIBC_M81_MATH_INLINES
#include <math.h>

#ifndef SUFF
#define SUFF
#endif
#ifndef float_type
#define float_type double
#endif

#define CONCATX(a,b) __CONCAT(a,b)
#define s(name) CONCATX(name,SUFF)
#define m81(func) __m81_u(s(func))

float_type
s(__modf) (float_type x, float_type *iptr)
{
  float_type x_int, result;
  __asm ("fintrz%.x %1, %0" : "=f" (x_int) : "f" (x));
  *iptr = x_int;
  if (m81(__isinf) (x))
    {
      result = 0;
      if (x < 0)
	result = -result;
    }
  else if (x == 0)
    result = x;
  else
    result = x - x_int;
  return result;
}

#define weak_aliasx(a,b) weak_alias(a,b)
weak_aliasx(s(__modf), s(modf))
