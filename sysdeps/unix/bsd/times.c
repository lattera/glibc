/* Copyright (C) 1991,92,93,95,96,97,1998,2001 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stddef.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <sys/time.h>
#include <time.h>


/* Time the program started.  */
extern time_t _posix_start_time;

#ifdef __GNUC__
__inline
#endif
static clock_t
timeval_to_clock_t (const struct timeval *tv, clock_t clk_tck)
{
  return (clock_t) ((tv->tv_sec * clk_tck) +
		    (tv->tv_usec * clk_tck / 1000000L));
}

/* Store the CPU time used by this process and all its
   dead children (and their dead children) in BUFFER.
   Return the elapsed real time, or (clock_t) -1 for errors.
   All times are in CLK_TCKths of a second.  */
clock_t
__times (buffer)
     struct tms *buffer;
{
  struct rusage usage;
  clock_t clk_tck;

  if (buffer == NULL)
    {
      __set_errno (EINVAL);
      return (clock_t) -1;
    }

  clk_tck = __getclktck ();
  
  if (__getrusage (RUSAGE_SELF, &usage) < 0)
    return (clock_t) -1;
  buffer->tms_utime = (clock_t) timeval_to_clock_t (&usage.ru_utime, clk_tck);
  buffer->tms_stime = (clock_t) timeval_to_clock_t (&usage.ru_stime, clk_tck);

  if (__getrusage (RUSAGE_CHILDREN, &usage) < 0)
    return (clock_t) -1;
  buffer->tms_cutime = (clock_t) timeval_to_clock_t (&usage.ru_utime, clk_tck);
  buffer->tms_cstime = (clock_t) timeval_to_clock_t (&usage.ru_stime, clk_tck);

  return (time ((time_t *) NULL) - _posix_start_time) * clk_tck;
}

weak_alias (__times, times)
