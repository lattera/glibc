/* futimens -- change access and modification times of open file.  Hurd version.
   Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

#include <sys/time.h>
#include <errno.h>
#include <stddef.h>
#include <hurd.h>
#include <hurd/fd.h>

/* Change the access time of FD to TSP[0] and
   the modification time of FD to TSP[1].  */
int
__futimens (int fd, const struct timespec tsp[2])
{
  time_value_t atime, mtime;
  error_t err;

  if (tsp == NULL)
    {
      /* Setting the number of microseconds to `-1' tells the
         underlying filesystems to use the current time.  */
      atime.microseconds = mtime.microseconds = -1;
    }
  else
    {
      atime.seconds = tsp[0].tv_sec;
      atime.microseconds = tsp[0].tv_nsec / 1000;
      mtime.seconds = tsp[1].tv_sec;
      mtime.microseconds = tsp[1].tv_nsec / 1000;
    }

  err = HURD_DPORT_USE (fd, __file_utimes (port, atime, mtime));
  return err ? __hurd_dfail (fd, err) : 0;
}
weak_alias (__futimens, futimens)
