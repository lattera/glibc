/* Copyright (C) 1992,1993,1994,1997,1999,2002 Free Software Foundation, Inc.
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
#include <string.h>
#include <sys/socket.h>
#include <hurd.h>
#include <hurd/fd.h>
#include <hurd/socket.h>

/* Await a connection on socket FD.
   When a connection arrives, open a new socket to communicate with it,
   set *ADDR (which is *ADDR_LEN bytes long) to the address of the connecting
   peer and *ADDR_LEN to the address's actual length, and return the
   new socket's descriptor, or -1 for errors.  */
int
accept (fd, addrarg, addr_len)
      int fd;
      __SOCKADDR_ARG addrarg;
      socklen_t *addr_len;
{
  error_t err;
  socket_t new;
  addr_port_t aport;
  struct sockaddr *addr = addrarg.__sockaddr__;
  char *buf = (char *) addr;
  mach_msg_type_number_t buflen;
  int type;

  if (err = HURD_DPORT_USE (fd, __socket_accept (port, &new, &aport)))
    return __hurd_dfail (fd, err);

  if (addr != NULL)
    {
      buflen = *addr_len;
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
      if (*addr_len > buflen)
	*addr_len = buflen;
      
      if (buf != (char *) addr)
	{
	  memcpy (addr, buf, *addr_len);
	  __vm_deallocate (__mach_task_self (), (vm_address_t) buf, buflen);
	}

      if (buflen > 0)
	addr->sa_family = type;
    }

  return _hurd_intern_fd (new, O_IGNORE_CTTY, 1);
}
libc_hidden_def (accept)
