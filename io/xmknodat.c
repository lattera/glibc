/* Copyright (C) 2005 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>

/* Create a device file named PATH relative to FD, with permission and
   special bits MODE and device number DEV (which can be constructed
   from major and minor device numbers with the `makedev' macro
   above).  */
int
__xmknodat (int vers, int fd, const char *path, mode_t mode, dev_t *dev)
{
  if (vers != _MKNOD_VER)
    {
      __set_errno (EINVAL);
      return -1;
    }

  if (path == NULL)
    {
      __set_errno (EINVAL);
      return -1;
    }

  if (fd != AT_FDCWD && path[0] != '/')
    {
      /* Check FD is associated with a directory.  */
      struct stat64 st;
      if (__fxstat64 (_STAT_VER, fd, &st) != 0)
	return -1;

      if (!S_ISDIR (st.st_mode))
	{
	  __set_errno (ENOTDIR);
	  return -1;
	}
    }

  __set_errno (ENOSYS);
  return -1;
}
stub_warning (__xmknodat)

libc_hidden_def (__xmknodat)
#include <stub-tag.h>
