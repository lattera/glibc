/* Copyright (C) 2011-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */


/* MicroBlaze uses socketcall.  */
#define __ASSUME_SOCKETCALL	1

/* All supported kernel versions for MicroBlaze have these syscalls.  */
#define __ASSUME_SOCKET_SYSCALL		1
#define __ASSUME_BIND_SYSCALL		1
#define __ASSUME_CONNECT_SYSCALL	1
#define __ASSUME_LISTEN_SYSCALL		1
#define __ASSUME_ACCEPT_SYSCALL		1
#define __ASSUME_GETSOCKNAME_SYSCALL	1
#define __ASSUME_GETPEERNAME_SYSCALL	1
#define __ASSUME_SOCKETPAIR_SYSCALL	1
#define __ASSUME_SEND_SYSCALL		1
#define __ASSUME_SENDTO_SYSCALL		1
#define __ASSUME_RECV_SYSCALL		1
#define __ASSUME_RECVFROM_SYSCALL	1
#define __ASSUME_SHUTDOWN_SYSCALL	1
#define __ASSUME_GETSOCKOPT_SYSCALL	1
#define __ASSUME_SETSOCKOPT_SYSCALL	1

/* Support for the accept4 and recvmmsg syscalls was added in 2.6.33.  */
#define __ASSUME_RECVMMSG_SYSCALL_WITH_SOCKETCALL      1

#include_next <kernel-features.h>

/* Support for the pselect6, preadv and pwritev syscalls was added in
   3.15.  */
#if __LINUX_KERNEL_VERSION < 0x030f00
# undef __ASSUME_PSELECT
# undef __ASSUME_PREADV
# undef __ASSUME_PWRITEV
#endif

/* Support for the sendmmsg syscall was added in 3.3.  */
#if __LINUX_KERNEL_VERSION < 0x030300
# undef __ASSUME_SENDMMSG_SYSCALL
#endif
