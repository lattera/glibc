/* Copyright (C) 1991,1995,1996,1997,2001,2002 Free Software Foundation, Inc.
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
#include <sys/socket.h>

/* Read N bytes into BUF through socket FD from peer
   at address ADDR (which is ADDR_LEN bytes long).
   Returns the number read or -1 for errors.  */
ssize_t
__recvfrom (fd, buf, n, flags, addr, addr_len)
     int fd;
     void *buf;
     size_t n;
     int flags;
     __SOCKADDR_ARG addr;
     socklen_t *addr_len;
{
  __set_errno (ENOSYS);
  return -1;
}

weak_alias (__recvfrom, recvfrom)

stub_warning (recvfrom)
#include <stub-tag.h>
