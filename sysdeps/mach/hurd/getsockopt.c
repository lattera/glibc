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

/* Put the current value for socket FD's option OPTNAME at protocol level LEVEL
   into OPTVAL (which is *OPTLEN bytes long), and set *OPTLEN to the value's
   actual length.  Returns 0 on success, -1 for errors.  */
int
DEFUN(getsockopt, (fd, level, optname, optval, optlen),
      int fd AND int level AND int optname AND
      PTR optval AND size_t *optlen)
{
  error_t err;
  char *buf = optval;
  mach_msg_type_number_t buflen = *optlen;

  if (err = HURD_DPORT_USE (fd, __socket_getopt (port,
						 level, optname,
						 &buf, &buflen)))
    return __hurd_dfail (fd, err);

  if (buf != optval)
    {
      if (*optlen < buflen)
	*optlen = buflen;
      memcpy (optval, buf, *optlen);
      __vm_deallocate (__mach_task_self (), (vm_address_t) buf, buflen);
    }

  return 0;
}
