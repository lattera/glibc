/* Copyright (C) 1998, 1999, 2001, 2002 Free Software Foundation, Inc.
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
#include <limits.h>
#include <stdlib.h>
#include <sys/statfs.h>

#include "linux_fsinfo.h"

/* Prototype for function that changes ownership and access permission
   for slave pseudo terminals that do not live on a `devpts'
   filesystem.  */
static int __unix_grantpt (int fd);

/* Prototype for private function that gets the name of the slave
   pseudo terminal in a safe way.  */
static int pts_name (int fd, char **pts, size_t buf_len);

/* Change the ownership and access permission of the slave pseudo
   terminal associated with the master pseudo terminal specified
   by FD.  */
int
grantpt (int fd)
{
  struct statfs fsbuf;
#ifdef PATH_MAX
  char _buf[PATH_MAX];
#else
  char _buf[512];
#endif
  char *buf = _buf;

  if (__builtin_expect (pts_name (fd, &buf, sizeof (_buf)), 0))
    {
      int save_errno = errno;

      /* Check, if the file descriptor is valid. pts_name returns the
	 wrong errno number, so we cannot use that.  */
      if (__libc_fcntl (fd, F_GETFD) == -1 && errno == EBADF)
	return -1;

       /* If the filedescriptor is no TTY, grantpt has to set errno
          to EINVAL.  */
       if (save_errno == ENOTTY)
         __set_errno (EINVAL);
       else
	 __set_errno (save_errno);

       return -1;
    }

  if (__statfs (buf, &fsbuf) < 0)
    return -1;

  /* If the slave pseudo terminal lives on a `devpts' filesystem, the
     ownership and access permission are already set.  */
  if (fsbuf.f_type == DEVPTS_SUPER_MAGIC || fsbuf.f_type == DEVFS_SUPER_MAGIC)
    return 0;

  return __unix_grantpt (fd);
}

#define grantpt static __unix_grantpt
#include <sysdeps/unix/grantpt.c>
