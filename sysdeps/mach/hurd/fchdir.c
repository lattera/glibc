/* Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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
#include <hurd.h>
#include <hurd/port.h>
#include <hurd/fd.h>
#include <fcntl.h>

/* Change the current directory to FD.  */
int
DEFUN(fchdir, (fd), int fd)
{
  error_t err;
  file_t dir;

  err = HURD_DPORT_USE (fd, (dir = __file_name_lookup_under (port, "",
							     O_EXEC, 0),
			     errno));

  if (! err)
    _hurd_port_set (&_hurd_ports[INIT_PORT_CWDIR], dir);

  return err ? __hurd_fail (err) : 0;
}
