/* Copyright (C) 2001, 2002 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sys/socket.h>
#include <sysdep-cancel.h>

/* Read N bytes into BUF from socket FD.
   Returns the number read or -1 for errors.  */

ssize_t
__libc_recv (int fd, void *buf, size_t n, int flags)
{
  if (SINGLE_THREAD_P)
    return INLINE_SYSCALL (recvfrom, 6, fd, buf, n, flags, NULL, NULL);

  int oldtype = LIBC_CANCEL_ASYNC ();

  ssize_t result = INLINE_SYSCALL (recvfrom, 6, fd, buf, n, flags, NULL, NULL);

  LIBC_CANCEL_RESET (oldtype);

  return result;
}

weak_alias (__libc_recv, __recv)
weak_alias (__recv, recv)
