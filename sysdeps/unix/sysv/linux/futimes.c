/* futimes -- change access and modification times of open file.  Linux version.
   Copyright (C) 2002-2015 Free Software Foundation, Inc.
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
#include <sysdep.h>
#include <string.h>
#include <time.h>
#include <utime.h>
#include <sys/time.h>
#include <_itoa.h>
#include <fcntl.h>


/* Change the access time of the file associated with FD to TVP[0] and
   the modification time of FILE to TVP[1].

   Starting with 2.6.22 the Linux kernel has the utimensat syscall which
   can be used to implement futimes.  */
int
__futimes (int fd, const struct timeval tvp[2])
{
  /* The utimensat system call expects timespec not timeval.  */
  struct timespec ts[2];
  if (tvp != NULL)
    {
      if (tvp[0].tv_usec < 0 || tvp[0].tv_usec >= 1000000
          || tvp[1].tv_usec < 0 || tvp[1].tv_usec >= 1000000)
	{
	  __set_errno (EINVAL);
	  return -1;
	}

      TIMEVAL_TO_TIMESPEC (&tvp[0], &ts[0]);
      TIMEVAL_TO_TIMESPEC (&tvp[1], &ts[1]);
    }

  return INLINE_SYSCALL (utimensat, 4, fd, NULL, tvp ? &ts : NULL, 0);
}
weak_alias (__futimes, futimes)
