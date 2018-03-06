/* futimesat -- Change access and modification times of file.  Linux version.
   Copyright (C) 2005-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <utime.h>
#include <sys/time.h>
#include <sysdep.h>


/* Change the access time of FILE relative to FD to TVP[0] and
   the modification time of FILE to TVP[1].  */
int
futimesat (int fd, const char *file, const struct timeval tvp[2])
{
  struct timespec tsp[2];
  int result;

  if (tvp)
    {
      if (tvp[0].tv_usec >= 1000000 || tvp[0].tv_usec < 0 ||
          tvp[1].tv_usec >= 1000000 || tvp[1].tv_usec < 0)
        {
          __set_errno (EINVAL);
          return -1;
        }
      TIMEVAL_TO_TIMESPEC (&tvp[0], &tsp[0]);
      TIMEVAL_TO_TIMESPEC (&tvp[1], &tsp[1]);
    }

  result = INLINE_SYSCALL (utimensat, 4, fd, file, tvp ? tsp : NULL, 0);
  return result;
}
