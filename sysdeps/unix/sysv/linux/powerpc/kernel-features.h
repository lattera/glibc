/* Set flags signalling availability of kernel features based on given
   kernel version number.  PowerPC version.
   Copyright (C) 1999-2015 Free Software Foundation, Inc.
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

/* PowerPC uses socketcall.  */
#define __ASSUME_SOCKETCALL		1

/* The accept4 syscall was added for PowerPC in 2.6.37.  */
#if __LINUX_KERNEL_VERSION >= 0x020625
# define __ASSUME_ACCEPT4_SYSCALL	1
#endif

/* The recvmmsg syscall was added for PowerPC in 2.6.37.  */
#if __LINUX_KERNEL_VERSION >= 0x020625
# define __ASSUME_RECVMMSG_SYSCALL	1
#endif

/* The sendmmsg syscall was added for PowerPC in 3.0.  */
#if __LINUX_KERNEL_VERSION >= 0x030000
# define __ASSUME_SENDMMSG_SYSCALL	1
#endif
#define __ASSUME_SENDMMSG_SYSCALL_WITH_SOCKETCALL	1

#include_next <kernel-features.h>

/* PowerPC64 IPC is always 64-bit and does not use __IPC_64.  */
#undef __ASSUME_IPC64
