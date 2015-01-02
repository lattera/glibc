/* Set flags signalling availability of kernel features based on given
   kernel version number.  i386 version.
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

/* i386 uses socketcall.  */
#define __ASSUME_SOCKETCALL		1

/* The recvmmsg syscall was added for i386 in 2.6.33.  */
#if __LINUX_KERNEL_VERSION >= 0x020621
# define __ASSUME_RECVMMSG_SYSCALL	1
#endif
#define __ASSUME_RECVMMSG_SYSCALL_WITH_SOCKETCALL	1

/* The sendmmsg syscall was added for i386 in 3.0.  */
#if __LINUX_KERNEL_VERSION >= 0x030000
# define __ASSUME_SENDMMSG_SYSCALL	1
#endif
#define __ASSUME_SENDMMSG_SYSCALL_WITH_SOCKETCALL	1

#include_next <kernel-features.h>
