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
#include <unistd.h>
#include <hurd.h>
#include <hurd/port.h>
#include <hurd/fd.h>

/* Change the current directory to FD.  */
int
DEFUN(fchdir, (fd), int fd)
{
  error_t err;
  file_t cwdir;

  err = __USEPORT (CRDIR,
		   ({ file_t crdir = port;
		      HURD_DPORT_USE (fd,
				      __hurd_file_name_lookup (crdir, port, "",
							       0, 0, &cwdir));
		    }));

  if (err)
    return __hurd_fail (err);

  _hurd_port_set (&_hurd_ports[INIT_PORT_CWDIR], cwdir);
  return 0;
}
