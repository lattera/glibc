/* nanosleep -- sleep for a period specified with a struct timespec
   Copyright (C) 2002 Free Software Foundation, Inc.
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
#include <mach.h>
#include <sys/time.h>
#include <unistd.h>

int
__nanosleep (const struct timespec *requested_time,
	     struct timespec *remaining)
{
  mach_port_t recv;
  struct timeval before, after;
  const mach_msg_timeout_t ms
    = requested_time->tv_sec * 1000
    + (requested_time->tv_nsec + 999999) / 1000000;

  recv = __mach_reply_port ();

  if (remaining && __gettimeofday (&before, NULL) < 0)
    return -1;
  (void) __mach_msg (NULL, MACH_RCV_MSG|MACH_RCV_TIMEOUT|MACH_RCV_INTERRUPT,
		     0, 0, recv, ms, MACH_PORT_NULL);
  __mach_port_destroy (mach_task_self (), recv);
  if (remaining && __gettimeofday (&after, NULL) < 0)
    return -1;

  if (remaining)
    {
      timersub (&after, &before, &after);
      TIMEVAL_TO_TIMESPEC (&after, remaining);
    }

  return 0;
}
libc_hidden_def (__nanosleep)
weak_alias (__nanosleep, nanosleep)
