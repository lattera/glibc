/* 128-bit floating point to 32-bit fixed point signed integer.
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

/* int _q_qtoi(const long double *a);
   Convert 'a' to signed int by truncation.

   Special cases:
   NaN: raise VXCVI, return 0x80000000
   SNaN: raise VXSNAN, VXCVI, and return 0x80000000
   >= 2^32: raise VXCVI, return 0x7fffffff
   <= -2^32: raise VXCVI, return 0x80000000
   */

int
__q_qtoi(const unsigned long long a[2])
{
  int ax, sgn;
  unsigned long long a0;

  a0 = a[0];
  /* 'sgn' is -1 if 'a' is negative, 0 if positive.  */
  sgn = (long long)a0 >> 63;
  ax = (a0 >> 48 & 0x7fff) - 16383;
  /* Deal with non-special cases.  */
  if (ax <= 30)
    /* If we had a '>>' operator that behaved properly with respect to
       large shifts, we wouldn't need the '& -(ax >> 31)' term, which
       clears 'result' if ax is negative.  The '^ sgn) - sgn' part is
       equivalent in this case to multiplication by 'sgn', but usually
       faster.  */
    return (((unsigned)(a0 >> 17  |  0x80000000) >> 31-ax  &  -(ax >> 31))
	    ^ sgn) - sgn;

  /* Check for SNaN, if so raise VXSNAN.  */
  if ((a0 >> 32  &  0x7fff8000) == 0x7fff0000
      && (a[1]  |  a0 & 0x00007fffffffffffULL) != 0)
    set_fpscr_bit(FPSCR_VXSNAN);

  /* Treat all NaNs as large negative numbers.  */
  sgn |= -cntlzw(~(a0 >> 32)  &  0x7fff0000) >> 5;

  /* All the specials raise VXCVI.  */
  set_fpscr_bit(FPSCR_VXCVI);

  /* Return 0x7fffffff (if a < 0) or 0x80000000 (otherwise).  */
  return sgn ^ 0x7fffffff;
}
