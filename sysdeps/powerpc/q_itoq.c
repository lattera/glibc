/* 32-bit fixed point signed integer to 128-bit floating point.
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

/* long double _q_itoq(int a);
   Convert 'a' to long double.
   */

void
__q_itoq(unsigned int result[4], int a)
{
  unsigned rx;
  int cs, css;

  /* Note that if a==-2^31, then ua will be 2^31.  */
  unsigned int ua = abs(a);

  /* Normalize.  */
  cs = cntlzw(ua);
  ua <<= cs+1;

  /* Calculate the exponent, in 4 easy instructions.  */
  css = 31-cs;
  rx = 16383+css << 16  &  ~css;
  
  /* Copy the sign bit from 'a'.  */
  asm ("rlwimi %0,%1,0,0,0": "=r"(rx) : "r"(a), "0"(rx));

  /* Put it all together.  */
  result[2] = result[3] = 0;
  asm ("rlwimi %0,%1,16,16,31": "=r"(result[0]) : "r"(ua), "0"(rx));
  result[1] = ua << 16;
}
