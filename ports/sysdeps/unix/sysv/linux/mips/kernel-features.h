/* Set flags signalling availability of kernel features based on given
   kernel version number.
   Copyright (C) 1999-2012 Free Software Foundation, Inc.
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

#include <sgidefs.h>

/* MIPS platforms had IPC64 all along.  */
#define __ASSUME_IPC64		1

/* MIPS had the utimes syscall by 2.6.0.  */
#define __ASSUME_UTIMES		1

/* Support for the eventfd2 and signalfd4 syscalls was added in 2.6.27.  */
#if __LINUX_KERNEL_VERSION >= 0x02061c
# define __ASSUME_EVENTFD2	1
# define __ASSUME_SIGNALFD4	1
#endif

#include_next <kernel-features.h>

/* The n32 syscall ABI did not have a getdents64 syscall until
   2.6.35.  */
#if _MIPS_SIM == _ABIN32 && __LINUX_KERNEL_VERSION < 0x020623
# undef __ASSUME_GETDENTS64_SYSCALL
#endif
