/* Set flags signalling availability of kernel features based on given
   kernel version number.
   Copyright (C) 2010-2012 Free Software Foundation, Inc.
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

#ifndef _KERNEL_FEATURES_H
#define _KERNEL_FEATURES_H 1

/* The utimes syscall has been available for some architectures
   forever.  */
#define __ASSUME_UTIMES	1

/* pselect/ppoll were introduced just after 2.6.16-rc1.  Due to the way
   the kernel versions are advertised we can only rely on 2.6.17 to have
   the code.  */
#if __LINUX_KERNEL_VERSION >= 0x020616
# define __ASSUME_PSELECT	1
# define __ASSUME_PPOLL		1
#endif

/* Support for various CLOEXEC and NONBLOCK flags was added in 2.6.23.  */
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

/* Support for the accept4 syscall was added in 3.3.  */
#if __LINUX_KERNEL_VERSION >= 0x030300
# define __ASSUME_ACCEPT4	1
#endif

#include_next <kernel-features.h>

#endif /* _KERNEL_FEATURES_H */
