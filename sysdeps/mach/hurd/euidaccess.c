/* Test for access to FILE using effective UID and GID.  Hurd version.
Copyright (C) 1991, 1995 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <hurd.h>

int
euidaccess (file, type)
     const char *file;
     int type;
{
  error_t err;
  file_t port;
  int allowed, flags;

  port = __file_name_lookup (file, 0, 0);
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
