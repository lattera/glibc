/* 128-bit floating point to 64-bit fixed point unsigned integer.
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

/* unsigned long long _q_qtoull(const long double *a);
   Convert 'a' to unsigned long long by truncation.

   Special cases:
   -0: return 0
   NaN: raise VXCVI, return 0
   SNaN: raise VXSNAN, VXCVI, and return 0
   Negative numbers (other than -0, but including -Inf): raise VXCVI, return 0
   >= 2^32: raise VXCVI, return 0xffffffffffffffff
   */

unsigned long long
__q_qtoull(const unsigned long long a[2])
{
  int ax;
  unsigned result;
  unsigned long long a0, a1;

  /* Deal with non-special cases.  */
  a0 = a[0]; a1 = a[1];
  ax = (a0 >> 48) - 16383;
  if (ax <= 63)
    /* If we had a '>>' operator that behaved properly with respect to
       large shifts, we wouldn't need the '& -(ax >> 31)' term, which
       clears 'result' if ax is negative.  */
    return ((a0 << 15  |  a1 >> 64-15  |  0x8000000000000000ULL) >> 63-ax
	    & -(unsigned long long)(ax >> 31));

  /* 'result' is 1 if a is negative, 0 otherwise.  */
  result = a0 >> 63;
  /* Check for -0, otherwise raise VXCVI.  */
  if (a0 != 0x8000000000000000ULL  ||  a[1] != 0)
  {
    /* Check for SNaN, if so raise VXSNAN.  */
    if ((a0 >> 32  &  0x7fff8000) == 0x7fff0000
	&& (a1  |  a0 & 0x00007fffffffffffULL) != 0)
      set_fpscr_bit(FPSCR_VXSNAN);
    /* Treat all NaNs as large negative numbers.  */
    result |= cntlzw(~(a0 >> 32)  &  0x7fff0000) >> 5;

    set_fpscr_bit(FPSCR_VXCVI);
  }
  return result-1LL;
}
