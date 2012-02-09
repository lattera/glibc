/* Copyright (C) 1991, 1997, 1998, 1999 Free Software Foundation, Inc.
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

#include <sys/resource.h>
#include <sys/time.h>
#include <time.h>

#ifdef	__GNUC__
__inline
#endif
static clock_t
timeval_to_clock_t (const struct timeval *tv)
{
  return (clock_t) ((tv->tv_sec * CLOCKS_PER_SEC) +
		    (tv->tv_usec * CLOCKS_PER_SEC / 1000000));
}

/* Return the time used by the program so far (user time + system time).  */
clock_t
clock (void)
{
  struct rusage usage;

  if (__getrusage (RUSAGE_SELF, &usage) < 0)
    return (clock_t) -1;

  return (timeval_to_clock_t (&usage.ru_stime) +
	  timeval_to_clock_t (&usage.ru_utime));
}
