/* Copyright (C) 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

/* Truncate PATH to LENGTH bytes.  */
int
DEFUN(truncate, (path, length),
      CONST char *path AND off_t length)
{
  int fd, ret, save;

  fd = open (path, O_WRONLY);
  if (fd < 0)
    return -1;

  ret = ftruncate (fd, length);
  save = errno;
  (void) close (fd);
  if (ret < 0)
    errno = save;
  return ret;
}
