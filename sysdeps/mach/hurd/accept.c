/* Copyright (C) 1992, 1993, 1994 Free Software Foundation, Inc.
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
#include <hurd.h>
#include <hurd/fd.h>
#include <sys/socket.h>
#include <hurd/socket.h>
#include <fcntl.h>
#include <string.h>

/* Await a connection on socket FD.
   When a connection arrives, open a new socket to communicate with it,
   set *ADDR (which is *ADDR_LEN bytes long) to the address of the connecting
   peer and *ADDR_LEN to the address's actual length, and return the
   new socket's descriptor, or -1 for errors.  */
int
DEFUN(accept, (fd, addr, addr_len),
      int fd AND struct sockaddr *addr AND size_t *addr_len)
{
  error_t err;
  socket_t new;
  addr_port_t aport;
  char *buf = (char *) addr;
  mach_msg_type_number_t buflen = *addr_len;
  int type;

  if (err = HURD_DPORT_USE (fd, __socket_accept (port, &new, &aport)))
    return __hurd_dfail (fd, err);

  if (addr != NULL)
    {
      err = __socket_whatis_address (aport, &type, &buf, &buflen);
      if (err == EOPNOTSUPP)
	/* If the protocol server can't tell us the address, just return a
	   zero-length one.  */
	{
	  buf = (char *)addr;
	  buflen = 0;
	  err = 0;
	}
    }
  __mach_port_deallocate (__mach_task_self (), aport);

  if (err)
    {
      __mach_port_deallocate (__mach_task_self (), new);
      return __hurd_dfail (fd, err);
    }

  if (addr != NULL)
    {
      if (buf != (char *) addr)
	{
	  if (*addr_len < buflen)
	    *addr_len = buflen;
	  memcpy (addr, buf, *addr_len);
	  __vm_deallocate (__mach_task_self (), (vm_address_t) buf, buflen);
	}

      if (buflen > 0)
	addr->sa_family = type;
    }

  return _hurd_intern_fd (new, O_IGNORE_CTTY, 1);
}
