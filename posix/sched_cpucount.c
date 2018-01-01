/* Copyright (C) 2007-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sched.h>


int
__sched_cpucount (size_t setsize, const cpu_set_t *setp)
{
  int s = 0;
  const __cpu_mask *p = setp->__bits;
  const __cpu_mask *end = &setp->__bits[setsize / sizeof (__cpu_mask)];

  while (p < end)
    {
      __cpu_mask l = *p++;

#ifdef POPCNT
      s += POPCNT (l);
#else
      if (l == 0)
	continue;

      _Static_assert (sizeof (l) == sizeof (unsigned int)
		      || sizeof (l) == sizeof (unsigned long)
		      || sizeof (l) == sizeof (unsigned long long),
		      "sizeof (__cpu_mask");
      if (sizeof (__cpu_mask) == sizeof (unsigned int))
	s += __builtin_popcount (l);
      else if (sizeof (__cpu_mask) == sizeof (unsigned long))
	s += __builtin_popcountl (l);
      else
	s += __builtin_popcountll (l);
#endif
    }

  return s;
}
