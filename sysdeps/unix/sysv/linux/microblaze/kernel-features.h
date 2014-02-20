/* Copyright (C) 2011-2014 Free Software Foundation, Inc.
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

/* MicroBlaze glibc support starts with 2.6.30, guaranteeing many kernel features.  */
#define __ASSUME_UTIMES         1
#define __ASSUME_O_CLOEXEC      1
#define __ASSUME_SOCK_CLOEXEC   1
#define __ASSUME_IN_NONBLOCK    1
#define __ASSUME_PIPE2          1
#define __ASSUME_EVENTFD2       1
#define __ASSUME_SIGNALFD4      1
#define __ASSUME_DUP3           1

/* Support for the accept4 and recvmmsg syscalls was added in 2.6.33.  */
#if __LINUX_KERNEL_VERSION >= 0x020621
# define __ASSUME_ACCEPT4_SYSCALL        1
# define __ASSUME_RECVMMSG_SYSCALL       1
#endif
#define __ASSUME_RECVMMSG_SYSCALL_WITH_SOCKETCALL      1

/* Support for the sendmmsg syscall was added in 3.3.  */
#if __LINUX_KERNEL_VERSION >= 0x030300
# define __ASSUME_SENDMMSG_SYSCALL       1
#endif

#include_next <kernel-features.h>
