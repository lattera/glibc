/* Copyright (C) 1991, 92, 93, 95, 96, 97, 98 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stddef.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/resource.h>


/* Time the program started.  */
extern time_t _posix_start_time;

#ifdef	__GNUC__
__inline
#endif
static clock_t
timeval_to_clock_t (const struct timeval *tv)
{
  return (clock_t) ((tv->tv_sec * CLK_TCK) +
		    (tv->tv_usec * CLK_TCK / 1000000L));
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

  if (buffer == NULL)
    {
      __set_errno (EINVAL);
      return (clock_t) -1;
    }

  if (__getrusage (RUSAGE_SELF, &usage) < 0)
    return (clock_t) -1;
  buffer->tms_utime = (clock_t) timeval_to_clock_t (&usage.ru_utime);
  buffer->tms_stime = (clock_t) timeval_to_clock_t (&usage.ru_stime);

  if (__getrusage (RUSAGE_CHILDREN, &usage) < 0)
    return (clock_t) -1;
  buffer->tms_cutime = (clock_t) timeval_to_clock_t (&usage.ru_utime);
  buffer->tms_cstime = (clock_t) timeval_to_clock_t (&usage.ru_stime);

  return (time ((time_t *) NULL) - _posix_start_time) * CLK_TCK;
}

weak_alias (__times, times)
