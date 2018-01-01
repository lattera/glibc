/* Set flags signalling availability of kernel features based on given
   kernel version number.  SH version.
   Copyright (C) 1999-2018 Free Software Foundation, Inc.
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

#ifndef __KERNEL_FEATURES_SH__
# define __KERNEL_FEATURES_SH__

/* SH uses socketcall.  */
#define __ASSUME_SOCKETCALL		1

/* These syscalls were added for SH in 2.6.37.  */
#define __ASSUME_SOCKET_SYSCALL		1
#define __ASSUME_BIND_SYSCALL		1
#define __ASSUME_CONNECT_SYSCALL	1
#define __ASSUME_LISTEN_SYSCALL		1
#define __ASSUME_GETSOCKNAME_SYSCALL	1
#define __ASSUME_GETPEERNAME_SYSCALL	1
#define __ASSUME_SOCKETPAIR_SYSCALL	1
#define __ASSUME_SEND_SYSCALL		1
#define __ASSUME_RECV_SYSCALL		1
#define __ASSUME_SHUTDOWN_SYSCALL	1
#define __ASSUME_GETSOCKOPT_SYSCALL	1
#define __ASSUME_SETSOCKOPT_SYSCALL	1

#include_next <kernel-features.h>

/* SH does not have a 64-bit inode field.  */
#undef __ASSUME_ST_INO_64_BIT
#define __ASSUME_ST_INO_64_BIT	0

/* SH4 ABI does not really require argument alignment for 64-bits, but
   the kernel interface for p{read,write}64 adds a dummy long argument
   before the offset.  */
#define __ASSUME_PRW_DUMMY_ARG	1

/* sh only supports ipc syscall.  */
#undef __ASSUME_DIRECT_SYSVIPC_SYSCALLS

#endif
