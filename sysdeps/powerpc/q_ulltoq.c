/* 64-bit fixed point unsigned integer to 128-bit floating point.
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

/* long double _q_ulltoq(unsigned long long a);
   Convert 'a' to long double.
   */

void
__q_ulltoq(unsigned int result[4], unsigned long long a)
{
  unsigned rx;
  int cs, css;

  /* Normalize.  */
  cs = cntlzd(a);
  a <<= cs+1;

  /* Calculate the exponent, in 4 easy instructions.  */
  css = 63-cs;
  rx = 16383+css << 16  &  ~css;

  /* Put it all together.  */
  result[3] = 0;
  result[0] = rx & 0xffff0000  |  a >> 48 & 0x0000ffff;
  a <<= 16;
  result[2] = a;
  result[1] = a >> 32;
}
