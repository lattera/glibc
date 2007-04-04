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

#include <limits.h>
#include <sched.h>


int
__sched_cpucount (size_t setsize, cpu_set_t *setp)
{
  int s = 0;
  for (unsigned int j = 0; j < setsize / sizeof (__cpu_mask); ++j)
    {
      __cpu_mask l = setp->__bits[j];
      if (l == 0)
	continue;

#if LONG_BIT > 32
      l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
      l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
      l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
      l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
      l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
      l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
#else
      l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
      l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
      l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
      l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
      l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
#endif

      s += l;
    }

  return s;
}
