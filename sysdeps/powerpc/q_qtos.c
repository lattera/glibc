/* 128-bit floating point to 32-bit floating point.
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

/* float _q_qtos(const long double *a);
   Convert 'a' to float. Round as per current rounding flags.

   Input       Rounding Output
   +/-0        *        +/-0
   +/-Inf      *        +/-Inf
   +/-NaN      *        +/-NaN (with mantissa truncated)
   +/-SNaN     *        +/-NaN (with mantissa truncated, MSB of mantissa <- 1)
                        && raise VXSNAN
			[Note: just truncating the mantissa may not give you
			a SNaN!]
   |a|>=2^128  Nearest  +/-Inf && raise overflow && raise inexact
   |a|>=2^128  Truncate +/-(2^128-2^104) && raise overflow && raise inexact
   a>=2^128    +Inf     +Inf && raise overflow && raise inexact
   a<=-2^128   +Inf     -(2^128-2^104) && raise overflow && raise inexact
   a>=2^128    -Inf     +(2^128-2^104) && raise overflow && raise inexact
   a<=-2^128   -Inf     -Inf && raise overflow && raise inexact
   
   We also need to raise 'inexact' if the result will be inexact, which
   depends on the current rounding mode.

   To avoid having to deal with all that, we convert to a 'double'
   that will round correctly (but is not itself rounded correctly),
   and convert that to a float.  This makes this procedure much
   simpler and much faster.  */

float
__q_qtos(const unsigned long long a[2])
{
  unsigned long long a0,d;
  union {
    double d;
    unsigned long long ull;
  } u;

  a0 = a[0];

  /* Truncate the mantissa to 48 bits.  */
  d = a0 << 4;
  /* Set the low bit in the mantissa if any of the bits we are dropping
     were 1.  This ensures correct rounding, and also distinguishes
     0 and Inf from denormalised numbers and SNaN (respectively).  */
  d |= a[1] != 0;
  /* Copy the sign bit.  */
  d = d & 0x7fffffffffffffffULL  |  a0 & 0x8000000000000000ULL;
  
  /* Now, we need to fix the exponent.  If the exponent of a was in
     the range +127 to -152, or was +16384 or -16383, it is already
     correct in 'd'.  Otherwise, we need to ensure that the new
     exponent is in the range +1023 to +128, or -153 to -1022, with
     the same sign as the exponent of 'a'.  We can do this by setting
     bits 1-3 (the second through fourth-most significant bit) of 'd'
     to 101 if bit 1 of 'a' is 1, or 010 if bit 1 of 'a' is 0.  */
  if ((a0 >> 56 & 0x7f) - 0x3f  >  1)
    {
      unsigned t = (a0 >> 32+2  &  2 << 31-1-2)*3 + (2 << 31-2);
      d = (d & 0x8fffffffffffffffULL
	   |  (unsigned long long)t<<32 & 0x7000000000000000ULL);
    }

  u.ull = d;
  return (float)u.d;
}
