/* Copyright (C) 1992, 1994 Free Software Foundation, Inc.
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

/* Put the local address of FD into *ADDR and its length in *LEN.  */
int
DEFUN(getsockname, (fd, addr, len),
      int fd AND struct sockaddr *addr AND size_t *len)
{
  error_t err;
  char *buf = (char *) addr;
  mach_msg_type_number_t buflen = *len;
  int type;
  addr_port_t aport;

  if (err = HURD_DPORT_USE (fd, __socket_name (port, &aport)))
    return __hurd_dfail (fd, err);

  err = __socket_whatis_address (aport, &type, &buf, &buflen);
  __mach_port_deallocate (__mach_task_self (), aport);

  if (err)
    return __hurd_dfail (fd, err);

  if (buf != (char *) addr)
    {
      if (*len < buflen)
	*len = buflen;
      memcpy (addr, buf, *len);
      __vm_deallocate (__mach_task_self (), (vm_address_t) buf, buflen);
    }

  addr->sa_family = type;

  return 0;
}
