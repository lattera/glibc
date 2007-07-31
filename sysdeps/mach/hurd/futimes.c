/* futimes -- change access and modification times of open file.  Hurd version.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sys/time.h>
#include <errno.h>
#include <stddef.h>
#include <hurd.h>
#include <hurd/fd.h>

/* Change the access time of FD to TVP[0] and
   the modification time of FD to TVP[1].  */
int
__futimes (int fd, const struct timeval tvp[2])
{
  struct timeval timevals[2];
  error_t err;

  if (tvp == NULL)
    {
      /* Setting the number of microseconds to `-1' tells the
         underlying filesystems to use the current time.  */
      timevals[1].tv_usec = timevals[0].tv_usec = (time_t)-1;
      tvp = timevals;
    }

  err = HURD_DPORT_USE (fd, __file_utimes (port,
					   *(time_value_t *) &tvp[0],
					   *(time_value_t *) &tvp[1]));
  return err ? __hurd_dfail (fd, err) : 0;
}
weak_alias (__futimes, futimes)
