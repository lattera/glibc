/* 32-bit floating point to 128-bit floating point.
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

/* long double _q_stoq(float a);
   Convert 'a' to long double. Don't raise exceptions.
   */

void
__q_stoq(unsigned long long result[2], float a)
{
  unsigned ux, rx;
  union {
    float d;
    unsigned u;
  } u;

  u.d = a;
  result[1] = 0;
  result[0] = u.u << 32-7;
  /* correct exponent bias */
  rx = ((int)u.u >> 23 & 0x80ff) + 16383-127;

  ux = u.u >> 23 & 0xff;
  if (ux == 0)
    {
      if ((u.u & 0x7fffffff) == 0)
	{
	  /* +0.0 or -0.0.  */
	  rx &= 0x8000;
	}
      else
	{
	  /* Denormalised number.  Renormalise.  */
	  unsigned um = u.u & 0x007fffff;
	  int cs = cntlzw(um) - 9 + 1;
	  rx -= cs;
	  um <<= cs;
	  result[0] = um << 32-7;
	}
    }
  else if (ux == 0xff)
    {
      /* Inf or NaN.  */
      rx |= 0x7fff;
    }
  *(unsigned short *)result = rx;
}
