/* Copyright (C) 2000 Free Software Foundation, Inc.
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

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statfs.h>

/* Reserve storage for the data of the file associated with FD.  */

int
posix_fallocate64 (int fd, __off64_t offset, size_t len)
{
  struct stat64 st;
  struct statfs64 f;
  size_t step;

  /* `off64_t´ is a signed type.  Therefore we can determine whether
     OFFSET + LEN is too large if it is a negative value.  */
  if (offset < 0 || len == 0)
    return EINVAL;
  if (offset + len < 0)
    return EFBIG;

  /* First thing we have to make sure is that this is really a regular
     file.  */
  if (__fxstat64 (_STAT_VER, fd, &st) != 0)
    return EBADF;
  if (S_ISFIFO (st.st_mode))
    return ESPIPE;
  if (! S_ISREG (st.st_mode))
    return ENODEV;

  /* We have to know the block size of the filesystem to get at least some
     sort of performance.  */
  if (__fstatfs64 (fd, &f) != 0)
    return errno;

  /* Align OFFSET to block size and adjust LEN.  */
  step = (offset + f.f_bsize - 1) % ~f.f_bsize;
  offset += step;

  /* Write something to every block.  */
  while (len > step)
    {
      len -= step;

      if (__pwrite64 (fd, "", 1, offset) != 1)
	return errno;

      offset += step;
    }

  return 0;
}
