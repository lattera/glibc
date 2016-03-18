/* Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

#include <sys/socket.h>
#include <sysdep-cancel.h>
#include <socketcall.h>
#include <shlib-compat.h>

ssize_t
__libc_recvmsg (int fd, struct msghdr *msg, int flags)
{
  ssize_t ret;

  /* POSIX specifies that both msghdr::msg_iovlen and msghdr::msg_controllen
     to be int and socklen_t respectively.  However Linux defines it as
     both size_t.  So for 64-bit it requires some adjustments by copying to
     temporary header and zeroing the pad fields.  */
#if __WORDSIZE == 64
  struct msghdr hdr, *orig = msg;
  if (msg != NULL)
    {
      hdr = *msg;
      hdr.__glibc_reserved1 = 0;
      hdr.__glibc_reserved2 = 0;
      msg = &hdr;
    }
#endif

#ifdef __ASSUME_RECVMSG_SYSCALL
  ret = SYSCALL_CANCEL (recvmsg, fd, msg, flags);
#else
  ret = SOCKETCALL_CANCEL (recvmsg, fd, msg, flags);
#endif

#if __WORDSIZE == 64
  if (orig != NULL)
    *orig = hdr;
#endif

  return ret;
}
weak_alias (__libc_recvmsg, __recvmsg)
versioned_symbol (libc, __libc_recvmsg, recvmsg, GLIBC_2_24);
