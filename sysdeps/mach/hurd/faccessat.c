/* Test for access to file, relative to open directory.  Hurd version.
   Copyright (C) 2006 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <sys/types.h>
#include <hurd.h>
#include <hurd/fd.h>

int
faccessat (fd, file, type, flag)
     int fd;
     const char *file;
     int type;
     int flag;
{
  error_t err;
  file_t port;
  int allowed, flags;

  if ((flag & AT_EACCESS) == 0)
    {
      if (fd == AT_FDCWD || file[0] == '/')
	return __access (file, type);
      __set_errno (ENOTSUP);	/* XXX later */
      return -1;
    }

  port = __file_name_lookup_at (fd, flag &~ AT_EACCESS, file, 0, 0);
  if (port == MACH_PORT_NULL)
    return -1;

  /* Find out what types of access we are allowed to this file.  */
  err = __file_check_access (port, &allowed);
  __mach_port_deallocate (__mach_task_self (), port);
  if (err)
    return __hurd_fail (err);

  flags = 0;
  if (type & R_OK)
    flags |= O_READ;
  if (type & W_OK)
    flags |= O_WRITE;
  if (type & X_OK)
    flags |= O_EXEC;

  if (flags & ~allowed)
    /* We are not allowed all the requested types of access.  */
    return __hurd_fail (EACCES);

  return 0;
}
