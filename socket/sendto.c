/* Copyright (C) 1991,1995-1997,2001,2002,2012 Free Software Foundation, Inc.
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

/* Send N bytes of BUF on socket FD to peer at address ADDR (which is
   ADDR_LEN bytes long).  Returns the number sent, or -1 for errors.  */
ssize_t
__sendto (fd, buf, n, flags, addr, addr_len)
     int fd;
     const __ptr_t buf;
     size_t n;
     int flags;
     __CONST_SOCKADDR_ARG addr;
     socklen_t addr_len;
{
  __set_errno (ENOSYS);
  return -1;
}

weak_alias (__sendto, sendto)

stub_warning (sendto)
#include <stub-tag.h>
