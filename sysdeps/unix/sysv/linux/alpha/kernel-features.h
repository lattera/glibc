/* Set flags signalling availability of kernel features based on given
   kernel version number.
   Copyright (C) 2010 Free Software Foundation, Inc.
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

/* alpha switched to a 64-bit timeval sometime before 2.2.0.  */
#if __LINUX_KERNEL_VERSION >= 131584
# define __ASSUME_TIMEVAL64	1
#endif

/* The tgkill syscall was introduced for alpha 2.6.0-test1 which unfortunately
   cannot be distinguished from 2.6.0.  */
#if __LINUX_KERNEL_VERSION >= 132609
# define __ASSUME_TGKILL	1
#endif

/* Starting with version 2.6.4, the stat64 syscalls are available.  */
#if __LINUX_KERNEL_VERSION >= 0x020604 && defined __alpha__
# define __ASSUME_STAT64_SYSCALL   1
#endif

#define __ASSUME_UTIMES	1

/* Starting with version 2.6.9, SSI_IEEE_RAISE_EXCEPTION exists.  */
#if __LINUX_KERNEL_VERSION >= 0x020609
# define __ASSUME_IEEE_RAISE_EXCEPTION	1
#endif

/* Support for the O_CLOEXEC flag was added for alpha in 2.6.23.  */
#if __LINUX_KERNEL_VERSION >= 0x020617
# define __ASSUME_O_CLOEXEC    1
#endif

/* Support for various CLOEXEC and NONBLOCK flags was added for alpha after
   2.6.33-rc1.  */
#if __LINUX_KERNEL_VERSION >= 0x020621
# define __ASSUME_SOCK_CLOEXEC  1
# define __ASSUME_IN_NONBLOCK   1
#endif

/* Support for the pipe2, eventfd2, signalfd4 syscalls was added for alpha 
   after 2.6.33-rc1.  */
#if __LINUX_KERNEL_VERSION >= 0x020621
# define __ASSUME_PIPE2     1
# define __ASSUME_EVENTFD2  1
# define __ASSUME_SIGNALFD4 1
#endif

/* Support for accept4 was added for alpha after 2.6.33-rc1.  */
#if __LINUX_KERNEL_VERSION >= 0x020621
# define __ASSUME_ACCEPT4      1
#endif

#include_next <kernel-features.h>

#undef __ASSUME_ST_INO_64_BIT

/* pselect/ppoll were introduced on alpha just after 2.6.22-rc1.  */
#if __LINUX_KERNEL_VERSION < 0x020617
# undef __ASSUME_PSELECT
# undef __ASSUME_PPOLL
#endif

/* The *at syscalls were introduced on alpha just after 2.6.22-rc1.  */
#if __LINUX_KERNEL_VERSION < 0x020617
# undef __ASSUME_ATFCTS
#endif

/* Support for inter-process robust mutexes was added on alpha just
   after 2.6.22-rc1.  */
#if __LINUX_KERNEL_VERSION < 0x020617
# undef __ASSUME_SET_ROBUST_LIST
#endif

/* Support for utimensat was added on alpha after 2.6.22-rc1.  */
#if __LINUX_KERNEL_VERSION < 0x020617
# undef __ASSUME_UTIMENSAT
#endif

/* Support for fallocate was added for alpha after 2.6.33-rc1.  */
#if __LINUX_KERNEL_VERSION < 0x020621
# undef __ASSUME_FALLOCATE
#endif
