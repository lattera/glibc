/* Copyright (C) 1991, 1995 Free Software Foundation, Inc.
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
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

/* Seek to OFFSET on FD, starting from WHENCE.  */
off_t
DEFUN(__lseek, (fd, offset, whence), int fd AND off_t offset AND int whence)
{
  if (fd < 0)
    {
      errno = EBADF;
      return -1;
    }
  switch (whence)
    {
    case SEEK_SET:
    case SEEK_CUR:
    case SEEK_END:
      break;
    default:
      errno = EINVAL;
      return -1;
    }

  errno = ENOSYS;
  return -1;
}
stub_warning (lseek)

weak_alias (__lseek, lseek)
