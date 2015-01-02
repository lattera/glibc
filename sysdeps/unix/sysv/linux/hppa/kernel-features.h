/* Set flags signalling availability of kernel features based on given
   kernel version number.
   Copyright (C) 2006-2015 Free Software Foundation, Inc.
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


/* PA-RISC 2.6.9 kernels had the first LWS CAS support */
#define __ASSUME_LWS_CAS		1

/* Support for the accept4 and recvmmsg syscalls was added in 2.6.34.  */
#if __LINUX_KERNEL_VERSION >= 0x020622
# define __ASSUME_ACCEPT4_SYSCALL	1
# define __ASSUME_RECVMMSG_SYSCALL	1
#endif

/* Support for the sendmmsg syscall was added in 3.1.  */
#if __LINUX_KERNEL_VERSION >= 0x030100
# define __ASSUME_SENDMMSG_SYSCALL	1
#endif

/* Support for the utimes syscall was added in 3.14.  */
#if __LINUX_KERNEL_VERSION >= 0x030e00
# define __ASSUME_UTIMES		1
#endif

#include_next <kernel-features.h>
