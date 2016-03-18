/* Compatibility version of recvmsg.
   Copyright (C) 2016 Free Software Foundation, Inc.
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

/* Both libc.so and libpthread.so provides sendmsg, so we need to
   provide the compat symbol for both libraries.  */
#if SHLIB_COMPAT (MODULE_NAME, GLIBC_2_0, GLIBC_2_24)

/* We can use the same struct layout for old symbol version since
   size is the same.  */
ssize_t
__old_recvmsg (int fd, struct msghdr *msg, int flags)
{
# ifdef __ASSUME_RECVMSG_SYSCALL
  return SYSCALL_CANCEL (recvmsg, fd, msg, flags);
# else
  return SOCKETCALL_CANCEL (recvmsg, fd, msg, flags);
# endif
}
compat_symbol (MODULE_NAME, __old_recvmsg, recvmsg, GLIBC_2_0);
#endif
