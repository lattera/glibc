/* Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
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

#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

/* Truncate PATH to LENGTH bytes.  */
int
truncate (path, length)
     const char *path;
     off_t length;
{
  int fd, ret, save;

  fd = open (path, O_WRONLY);
  if (fd < 0)
    return -1;

  ret = ftruncate (fd, length);
  save = errno;
  (void) close (fd);
  if (ret < 0)
    __set_errno (save);
  return ret;
}
