/* Copyright (C) 1991, 1992, 1993, 1994 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <unistd.h>
#include <hurd.h>
#include <fcntl.h>
#include <hurd/port.h>

/* Change the current root to FILE_NAME.  */
int
DEFUN(chroot, (file_name), CONST char *file_name)
{
  file_t file, dir;
  error_t err;

  file = __file_name_lookup (file_name, O_EXEC, 0);
  if (file == MACH_PORT_NULL)
    return -1;
  err = __USEPORT (CRDIR, __hurd_file_name_lookup (port, file, "",
						   O_EXEC, 0, &dir));
  __mach_port_deallocate (__mach_task_self (), file);
  if (err)
    return __hurd_fail (err);

  _hurd_port_set (&_hurd_ports[INIT_PORT_CRDIR], dir);
  return 0;
}
