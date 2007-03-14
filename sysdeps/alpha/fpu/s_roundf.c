/* Copyright (C) 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <math.h>


float
__roundf (float x)
{
  const float almost_half = 0x1.fffffep-2;
  const float two23 = 0x1.0p23;
  float r, tmp;
  
  __asm (
#ifdef _IEEE_FP_INEXACT
	 "adds/suic %2, %3, %1\n\tsubs/suic %1, %3, %0"
#else
	 "adds/suc %2, %3, %1\n\tsubs/suc %1, %3, %0"
#endif
	 : "=&f"(r), "=&f"(tmp)
	 : "f"(fabsf (x) + almost_half), "f"(two23));

  /* round(-0) == -0, and in general we'll always have the same
     sign as our input.  */
  return copysignf (r, x);
}

weak_alias (__roundf, roundf)
