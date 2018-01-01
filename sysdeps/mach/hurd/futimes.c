/* futimes -- change access and modification times of open file.  Hurd version.
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

/* Change the access time of FD to TVP[0] and
   the modification time of FD to TVP[1].  */
int
__futimes (int fd, const struct timeval tvp[2])
{
  union tv
  {
    struct timeval tv;
    time_value_t tvt;
  };
  const union tv *u = (const union tv *) tvp;
  union tv nulltv[2];
  error_t err;

  if (tvp == NULL)
    {
      /* Setting the number of microseconds to `-1' tells the
         underlying filesystems to use the current time.  */
      nulltv[0].tvt.microseconds = nulltv[1].tvt.microseconds = -1;
      u = nulltv;
    }

  err = HURD_DPORT_USE (fd, __file_utimes (port, u[0].tvt, u[1].tvt));
  return err ? __hurd_dfail (fd, err) : 0;
}
weak_alias (__futimes, futimes)
