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
  struct timespec atime, mtime;
  error_t err;

  if (tsp == NULL)
    {
      /* Setting the number of nanoseconds to UTIME_NOW tells the
         underlying filesystems to use the current time.  */
      atime.tv_sec = 0;
      atime.tv_nsec = UTIME_NOW;
      mtime.tv_sec = 0;
      mtime.tv_nsec = UTIME_NOW;
    }
  else
    {
      atime = tsp[0];
      mtime = tsp[1];
    }

  err = HURD_DPORT_USE (fd, __file_utimens (port, atime, mtime));

  if (err == MIG_BAD_ID || err == EOPNOTSUPP)
    {
      time_value_t atim, mtim;

      if (tsp == NULL)
        /* Setting the number of microseconds to `-1' tells the
           underlying filesystems to use the current time.  */
        atim.microseconds = mtim.microseconds = -1;
      else
        {
          if (tsp[0].tv_nsec == UTIME_NOW)
            atim.microseconds = -1;
          else if (tsp[0].tv_nsec == UTIME_OMIT)
            atim.microseconds = -2;
          else
            TIMESPEC_TO_TIME_VALUE (&atim, &(tsp[0]));
          if (tsp[1].tv_nsec == UTIME_NOW)
            mtim.microseconds = -1;
          else if (tsp[1].tv_nsec == UTIME_OMIT)
            mtim.microseconds = -2;
          else
            TIMESPEC_TO_TIME_VALUE (&mtim, &(tsp[1]));
        }

      err = HURD_DPORT_USE (fd, __file_utimes (port, atim, mtim));
  }

  return err ? __hurd_dfail (fd, err) : 0;
}
weak_alias (__futimens, futimens)
