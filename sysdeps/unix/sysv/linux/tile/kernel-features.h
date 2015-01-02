/* Copyright (C) 2011-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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


/* TILE glibc support starts with 2.6.36, guaranteeing many kernel features. */
#define __ASSUME_ACCEPT4_SYSCALL	1
#define __ASSUME_RECVMMSG_SYSCALL	1

/* Support for the sendmmsg syscall was added in 3.0.  */
#if __LINUX_KERNEL_VERSION >= 0x030000
# define __ASSUME_SENDMMSG_SYSCALL	1
#endif

#include_next <kernel-features.h>

/* Define this if your 32-bit syscall API requires 64-bit register
   pairs to start with an even-number register.  */
#define __ASSUME_ALIGNED_REGISTER_PAIRS	1
