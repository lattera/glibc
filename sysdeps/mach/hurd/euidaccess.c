/* Test for access to FILE using effective UID and GID.  Hurd version.
   Copyright (C) 1991, 1995, 1997, 2006 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <hurd.h>

int
__euidaccess (file, type)
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
weak_alias (__euidaccess, euidaccess)
weak_alias (__euidaccess, eaccess)
