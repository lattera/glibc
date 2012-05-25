/* Set flags signalling availability of kernel features based on given
   kernel version number.
   Copyright (C) 2008, 2009, 2012 Free Software Foundation, Inc.
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

/* These features were surely available with 2.4.12.  */
#if __LINUX_KERNEL_VERSION >= 132108
# define __ASSUME_MMAP2_SYSCALL		1
# define __ASSUME_TRUNCATE64_SYSCALL	1
# define __ASSUME_STAT64_SYSCALL	1
# define __ASSUME_FCNTL64		1
# define __ASSUME_VFORK_SYSCALL		1
#endif

/* Many syscalls were added in 2.6.10 for m68k.  */
#if __LINUX_KERNEL_VERSION >= 132618
# define __ASSUME_TGKILL	1
# define __ASSUME_UTIMES	1
# define __ASSUME_FADVISE64_64_SYSCALL	1
#endif

#include_next <kernel-features.h>

/* These syscalls were added only in 3.0 for m68k.  */
#if __LINUX_KERNEL_VERSION < 0x030000
# undef __ASSUME_PSELECT
# undef __ASSUME_PPOLL
#endif
