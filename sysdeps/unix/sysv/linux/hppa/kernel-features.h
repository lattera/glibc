/* Set flags signalling availability of kernel features based on given
   kernel version number.
   Copyright (C) 2006 Free Software Foundation, Inc.
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


/* There are an infinite number of PA-RISC kernel versions numbered
   2.4.0.  But they've not really been released as such.  We require
   and expect the final version here.  */
#define __ASSUME_32BITUIDS		1
#define __ASSUME_TRUNCATE64_SYSCALL	1
#define __ASSUME_MMAP2_SYSCALL		1
#define __ASSUME_STAT64_SYSCALL		1
#define __ASSUME_IPC64			1
#define __ASSUME_ST_INO_64_BIT		1
#define __ASSUME_FCNTL64		1
#define __ASSUME_GETDENTS64_SYSCALL	1

/* PA-RISC 2.6.9 kernels had the first LWS CAS support */
#if __LINUX_KERNEL_VERSION >= 0x020609
# define __ASSUME_LWS_CAS		1
#endif

#include_next <kernel-features.h>
