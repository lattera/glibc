/* Copyright (C) 1994 Free Software Foundation, Inc.
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
#include <sys/socket.h>
#include <hurd.h>
#include <hurd/socket.h>
#include <hurd/fd.h>

/* Send N bytes of BUF to socket FD.  Returns the number sent or -1.  */
int
DEFUN(send, (fd, buf, n, flags),
      int fd AND PTR buf AND size_t n AND int flags)
{
  error_t err;
  int wrote;

  err = HURD_DPORT_USE (fd, __socket_send (port, MACH_PORT_NULL,
					   flags, buf, n,
					   NULL, MACH_MSG_TYPE_COPY_SEND, 0,
					   NULL, 0, &wrote));

  return err ? __hurd_dfail (fd, err) : wrote;
}
