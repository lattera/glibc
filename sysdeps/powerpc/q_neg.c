/* Negate a 128-bit floating point value.
   Copyright (C) 1997 Free Software Foundation, Inc.
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

#include <quad_float.h>

/* long double _q_neg(const long double *a);
   Negate 'a'. Don't raise exceptions.
   */

void
__q_neg(unsigned int result[4], unsigned int a[4])
{
  unsigned int t1,t2;
  t1 = a[0];
  t2 = a[1];
  result[0] = t1 ^ 0x80000000;
  t1 = a[2];
  result[1] = t2;
  t2 = a[3];
  result[2] = t1;
  result[3] = t2;
}
