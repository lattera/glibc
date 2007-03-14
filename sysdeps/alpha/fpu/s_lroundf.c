/* Copyright (C) 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#define __llroundf	not___llroundf
#define llroundf	not_llroundf
#include <math.h>
#undef __llroundf
#undef llroundf


long int
__lroundf (float x)
{
  float adj;

  adj = 0x1.fffffep-2;		/* nextafterf (0.5f, 0.0f) */
  adj = copysignf (adj, x);
  return x + adj;
}

strong_alias (__lroundf, __llroundf)
weak_alias (__lroundf, lroundf)
weak_alias (__llroundf, llroundf)
