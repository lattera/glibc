/* Inline math functions for Alpha.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger-Tang.

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

#if defined (__GNUC__) && !defined (__NO_MATH_INLINES)

extern __inline double
__copysign (double __x, double __y)
{
  __asm ("cpys %1, %2, %0" : "=f" (__x) : "f" (__y), "f" (__x));
  return __x;
}

extern __inline double
fabs (double __x)
{
  __asm ("cpys $f31, %1, %0" : "=f" (__x) : "f" (__x));
  return __x;
}

extern __inline double
atan (double __x)
{
  extern double __atan2 (double, double);
  return __atan2 (__x, 1.0);
}

#endif
