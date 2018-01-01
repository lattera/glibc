/* Copyright (C) 1992-2018 Free Software Foundation, Inc.
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

/* Sleep USECONDS microseconds, or until a previously set timer goes off.  */
int
usleep (useconds_t useconds)
{
  mach_port_t recv;
  struct timeval before, after;

  recv = __mach_reply_port ();

  if (__gettimeofday (&before, NULL) < 0)
    return -1;
  (void) __mach_msg (NULL, MACH_RCV_MSG|MACH_RCV_TIMEOUT|MACH_RCV_INTERRUPT,
		     0, 0, recv, (useconds + 999) / 1000, MACH_PORT_NULL);
  __mach_port_destroy (mach_task_self (), recv);
  if (__gettimeofday (&after, NULL) < 0)
    return -1;

  return 0;
}
