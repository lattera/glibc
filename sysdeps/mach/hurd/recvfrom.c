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
#include <hurd/fd.h>
#include <hurd/socket.h>
#include <string.h>

/* Read N bytes into BUF through socket FD from peer
   at address ADDR (which is ADDR_LEN bytes long).
   Returns the number read or -1 for errors.  */
int
DEFUN(recvfrom, (fd, buf, n, flags, addr, addr_len),
      int fd AND PTR buf AND size_t n AND int flags AND
      struct sockaddr *addr AND size_t *addr_len)
{
  error_t err;
  mach_port_t addrport;
  char *bufp = buf;
  mach_msg_type_number_t nread = n;
  mach_port_t *ports;
  mach_msg_type_number_t nports;
  char *cdata = NULL;
  mach_msg_type_number_t clen = 0;

  if (err = HURD_DPORT_USE (fd, __socket_recv (port, &addrport,
					       flags, &bufp, &nread,
					       &ports, &nports,
					       &cdata, &clen,
					       &flags,
					       n)))
    return __hurd_dfail (fd, err);

  /* Get address data for the returned address port.  */
  {
    char *buf = (char *) addr;
    mach_msg_type_number_t buflen = *addr_len;
    int type;

    err = __socket_whatis_address (addrport, &type, &buf, &buflen);
    if (err == EOPNOTSUPP)
      /* If the protocol server can't tell us the address, just return a
	 zero-length one.  */
      {
	buf = (char *)addr;
	buflen = 0;
	err = 0;
      }
    __mach_port_deallocate (__mach_task_self (), addrport);
    if (err)
      return __hurd_dfail (fd, err);

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

  /* Toss control data; we don't care.  */
  __vm_deallocate (__mach_task_self (), (vm_address_t) cdata, clen);

  if (bufp != buf)
    {
      memcpy (buf, bufp, nread);
      __vm_deallocate (__mach_task_self (), (vm_address_t) bufp, nread);
    }

  return nread;
}

