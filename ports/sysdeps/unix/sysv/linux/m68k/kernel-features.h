/* Set flags signalling availability of kernel features based on given
   kernel version number.
   Copyright (C) 2008-2014 Free Software Foundation, Inc.
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

/* Many syscalls were added in 2.6.10 for m68k.  */
#define __ASSUME_UTIMES	1

/* Support for various CLOEXEC and NONBLOCK flags was added 2.6.23.  */
#if __LINUX_KERNEL_VERSION >= 0x020617
# define __ASSUME_O_CLOEXEC	1
#endif

/* Support for various CLOEXEC and NONBLOCK flags was added in 2.6.27.  */
#if __LINUX_KERNEL_VERSION >= 0x02061b
# define __ASSUME_SOCK_CLOEXEC	1
# define __ASSUME_IN_NONBLOCK	1
# define __ASSUME_PIPE2		1
# define __ASSUME_EVENTFD2	1
# define __ASSUME_SIGNALFD4	1
# define __ASSUME_DUP3		1
#endif

/* Support for the accept4 syscall was added in 2.6.28.  */
#if __LINUX_KERNEL_VERSION >= 0x02061c
# define __ASSUME_ACCEPT4	1
#endif

#include_next <kernel-features.h>

/* These syscalls were added only in 3.0 for m68k.  */
#if __LINUX_KERNEL_VERSION < 0x030000
# undef __ASSUME_PSELECT
# undef __ASSUME_PPOLL
#endif

/* No support for PI futexes or robust mutexes before 3.10 for m68k.  */
#if __LINUX_KERNEL_VERSION < 0x030a00
# undef __ASSUME_REQUEUE_PI
# undef __ASSUME_SET_ROBUST_LIST
#endif
