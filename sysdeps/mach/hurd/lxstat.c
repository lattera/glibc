/* Copyright (C) 1992, 1993, 1994, 1995, 1996 Free Software Foundation, Inc.
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
#include <sys/stat.h>
#include <stddef.h>
#include <fcntl.h>
#include <hurd.h>

int
__lxstat (int vers, const char *file, struct stat *buf)
{
  error_t err;
  file_t port;

  if (vers != _STAT_VER)
    return __hurd_fail (EINVAL);

  port = __file_name_lookup (file, O_NOLINK, 0);
  if (port == MACH_PORT_NULL)
    return -1;
  err = __io_stat (port, buf);
  __mach_port_deallocate (__mach_task_self (), port);
  if (err)
    return __hurd_fail (err);
  return 0;
}

weak_alias (__lxstat, _lxstat)
