/* 128-bit floating point to 64-bit fixed point signed integer.
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

/* long long _q_qtoll(const long double *a);
   Convert 'a' to signed long long by truncation.

   Special cases:
   NaN: raise VXCVI, return 0x8000000000000000
   SNaN: raise VXSNAN, VXCVI, and return 0x8000000000000000
   >= 2^32: raise VXCVI, return 0x7fffffffffffffff
   <= -2^32: raise VXCVI, return 0x8000000000000000
   */

long long
__q_qtoll(const unsigned long long a[2])
{
  int ax, sgn;
  unsigned long long a0, a1;

  a0 = a[0]; a1 = a[1];
  /* 'sgn' is -1 if 'a' is negative, 0 if positive.  */
  sgn = (long long)a0 >> 63;
  ax = (a0 >> 48 & 0x7fff) - 16383;
  /* Deal with non-special cases.  */
  if (ax <= 62)
    /* If we had a '>>' operator that behaved properly with respect to
       large shifts, we wouldn't need the '& -(ax >> 31)' term, which
       clears 'result' if ax is negative.  The '^ sgn) - sgn' part is
       equivalent in this case to multiplication by 'sgn', but faster.  */
    return (((a0 << 15  |  a1 >> 64-15  |  0x8000000000000000ULL) >> 63-ax
	     & -(unsigned long long)(ax >> 31))
	    ^  (long long)sgn) - sgn;

  /* Check for SNaN, if so raise VXSNAN.  */
  if ((a0 >> 32  &  0x7fff8000) == 0x7fff0000
      && (a1  |  a0 & 0x00007fffffffffffULL) != 0)
    set_fpscr_bit(FPSCR_VXSNAN);

  /* Treat all NaNs as large negative numbers.  */
  sgn |= -cntlzw(~(a0 >> 32)  &  0x7fff0000) >> 5;

  /* All the specials raise VXCVI.  */
  set_fpscr_bit(FPSCR_VXCVI);

  return (long long)sgn ^ 0x7fffffffffffffffLL;
}
