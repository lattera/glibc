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

/* Send N bytes of BUF on socket FD to peer at address ADDR (which is
   ADDR_LEN bytes long).  Returns the number sent, or -1 for errors.  */
int
DEFUN(sendto, (fd, buf, n, flags, addr, addr_len),
      int fd AND PTR buf AND size_t n AND int flags AND
      struct sockaddr *addr AND size_t addr_len)
{
  addr_port_t aport;
  error_t err;
  int wrote;

  /* Get an address port for the desired destination address.  */
  err = HURD_DPORT_USE (fd,
			({
			  err = __socket_create_address (port,
							 addr->sa_family,
							 (char *) addr,
							 addr_len,
							 &aport, 1);
			  if (! err)
			    {
			      /* Send the data.  */
			      err = __socket_send (port, aport,
						   flags, buf, n,
						   NULL,
						   MACH_MSG_TYPE_COPY_SEND, 0,
						   NULL, 0, &wrote);
			      __mach_port_deallocate (__mach_task_self (),
						      aport);
			    }
			  err;
			}));

  return err ? __hurd_dfail (fd, err) : wrote;
}
