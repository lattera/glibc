/* 64-bit floating point to 128-bit floating point.
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

/* long double _q_dtoq(double a);
   Convert 'a' to long double. Don't raise exceptions.
   */

void
__q_dtoq(unsigned long long result[2], double a)
{
  unsigned ux, rx;
  union {
    double d;
    unsigned long long u;
  } u;

  u.d = a;
  result[1] = u.u << 64-4;
  result[0] = u.u >> 4;
  /* correct exponent bias */
  rx = ((long long)u.u >> 52 & 0x87ff) + 16383-1023;

  ux = u.u >> 52 & 0x7ff;
  if (ux == 0)
    {
      if ((u.u & 0x7fffffffffffffffULL) == 0)
	{
	  /* +0.0 or -0.0.  */
	  rx &= 0x8000;
	}
      else
	{
	  /* Denormalised number.  Renormalise.  */
	  unsigned long long um = u.u & 0x000fffffffffffffULL;
	  int cs = cntlzd(um) - 12 + 1;
	  rx -= cs;
	  um <<= cs;
	  result[0] = um >> 4;
	  result[1] = um << 64-4;
	}
    }
  else if (ux == 0x7ff)
    {
      /* Inf or NaN.  */
      rx |= 0x7fff;
    }
  *(unsigned short *)result = rx;
}
