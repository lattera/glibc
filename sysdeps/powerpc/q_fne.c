/* 128-bit floating point unordered not-equality.
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

#include <fenv_libc.h>

/* int _q_fne(const long double *a, const long double *b);
   Returns 0 if a==b

   Special cases:
   NaNs return 1 always;
   -0 == +0;
   If either argument is a SNaN, we set FPSCR_VXSNAN to cause an
   'invalid operation' exception.
   */

int
__q_fne(const unsigned a[4], const unsigned b[4])
{
  unsigned a0, b0;

  a0 = a[0]; b0 = b[0];

  if ((a0 >> 16  &  0x7fff) != 0x7fff &&
      (b0 >> 16  &  0x7fff) != 0x7fff)
    {
      unsigned a3,b3,a2,b2,a1,b1;
      unsigned z, result;

      a3 = a[3]; b3 = b[3]; a2 = a[2];
      result = a3 ^ b3;
      b2 = b[2];
      if (result != 0)
	return result;
      a1 = a[1]; b1 = b[1];
      result = a2 ^ b2;
      result |= a1 ^ b1;
      z = a3 | a2;
      if (result != 0)
	return result;
      z |= a1;
      z = (z >> 1 | z | a0) & 0x7fffffff;
      /* 'z' is 0 iff a==0.0, otherwise between 1 and 0x7fffffff */
      return (a0^b0) & ~(-z & 0x80000000);
    }
  else
    {
      /* Deal with SNaNs */
      if (((a0|b0) & 0x8000) == 0)
	{
	  set_fpscr_bit(FPSCR_VXSNAN);
	}
      return 1;
    }
}
