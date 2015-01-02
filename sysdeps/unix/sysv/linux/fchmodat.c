/* Change the protections of file relative to open directory.  Linux version.
   Copyright (C) 2006-2015 Free Software Foundation, Inc.
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
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <alloca.h>
#include <sysdep.h>

int
fchmodat (fd, file, mode, flag)
     int fd;
     const char *file;
     mode_t mode;
     int flag;
{
  if (flag & ~AT_SYMLINK_NOFOLLOW)
    {
      __set_errno (EINVAL);
      return -1;
    }
#ifndef __NR_lchmod		/* Linux so far has no lchmod syscall.  */
  if (flag & AT_SYMLINK_NOFOLLOW)
    {
      __set_errno (ENOTSUP);
      return -1;
    }
#endif

  return INLINE_SYSCALL (fchmodat, 3, fd, file, mode);
}
