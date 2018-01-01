/* Return information about the filesystem on which FD resides.
   Copyright (C) 1996-2018 Free Software Foundation, Inc.
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
#include <string.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include "internal_statvfs.h"
#include <kernel-features.h>

/* Return information about the filesystem on which FD resides.  */
int
__fstatvfs64 (int fd, struct statvfs64 *buf)
{
  struct statfs64 fsbuf;
  int res = __fstatfs64 (fd, &fsbuf);

#ifndef __ASSUME_STATFS64
  if (res < 0 && errno == ENOSYS)
    {
      struct statvfs buf32;

      res = fstatvfs (fd, &buf32);
      if (res == 0)
	{
	  buf->f_bsize = buf32.f_bsize;
	  buf->f_frsize = buf32.f_frsize;
	  buf->f_blocks = buf32.f_blocks;
	  buf->f_bfree = buf32.f_bfree;
	  buf->f_bavail = buf32.f_bavail;
	  buf->f_files = buf32.f_files;
	  buf->f_ffree = buf32.f_ffree;
	  buf->f_favail = buf32.f_favail;
	  buf->f_fsid = buf32.f_fsid;
	  buf->f_flag = buf32.f_flag;
	  buf->f_namemax = buf32.f_namemax;
	  memcpy (buf->__f_spare, buf32.__f_spare, sizeof (buf32.__f_spare));
	}
    }
#endif

  if (res == 0)
    /* Convert the result.  */
    __internal_statvfs64 (NULL, buf, &fsbuf, fd);

  return res;
}
weak_alias (__fstatvfs64, fstatvfs64)
