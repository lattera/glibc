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

/* The utimes syscall was added before 2.6.1.  */
#if __LINUX_KERNEL_VERSION >= 132609
# define __ASSUME_UTIMES	1
#endif

/* The new getrlimit syscall was added sometime before 2.4.6.  */
#if __LINUX_KERNEL_VERSION >= 132102
#define __ASSUME_NEW_GETRLIMIT_SYSCALL	1
#endif

/* On ARM the truncate64/ftruncate64/mmap2/stat64/lstat64/fstat64
   syscalls were introduced in 2.3.35.  */
#if __LINUX_KERNEL_VERSION >= 131875
# define __ASSUME_TRUNCATE64_SYSCALL	1
# define __ASSUME_MMAP2_SYSCALL		1
# define __ASSUME_STAT64_SYSCALL	1
#endif

/* Arm got fcntl64 in 2.4.4.  */
#if __LINUX_KERNEL_VERSION >= 132100
# define __ASSUME_FCNTL64		1
#endif

/* The vfork syscall on arm was definitely available in 2.4.  */
#if __LINUX_KERNEL_VERSION >= 132097
# define __ASSUME_VFORK_SYSCALL		1
#endif

/* The signal frame layout changed in 2.6.18.  */
#if __LINUX_KERNEL_VERSION >= 132626
# define __ASSUME_SIGFRAME_V2	1
#endif

/* Support for the eventfd2 and signalfd4 syscalls was added in 2.6.27.  */
#if __LINUX_KERNEL_VERSION >= 0x02061b
# define __ASSUME_EVENTFD2	1
# define __ASSUME_SIGNALFD4	1
#endif

#include_next <kernel-features.h>

/* Support for pselect6, ppoll and epoll_pwait was added in 2.6.32.  */
#if __LINUX_KERNEL_VERSION < 0x020620
# undef __ASSUME_PSELECT
# undef __ASSUME_PPOLL
#endif
