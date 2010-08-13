/* Set flags signalling availability of kernel features based on given
   kernel version number.
   Copyright (C) 1999-2003, 2004, 2005, 2006 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sgidefs.h>

/* Linux 2.3.39 introduced 32bit UID/GIDs.  Some platforms had 32
   bit type all along.  */
#define __ASSUME_32BITUIDS		1

/* MIPS platforms had IPC64 all along.  */
#define __ASSUME_IPC64		1

#if _MIPS_SIM == _ABIN32
# define __ASSUME_FCNTL64		1
#endif

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
