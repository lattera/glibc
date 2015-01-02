/* Set flags signalling availability of kernel features based on given
   kernel version number.
   Copyright (C) 2008-2015 Free Software Foundation, Inc.
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

/* m68k uses socketcall.  */
#define __ASSUME_SOCKETCALL	1

#include_next <kernel-features.h>

/* These syscalls were added only in 3.0 for m68k.  */
#if __LINUX_KERNEL_VERSION < 0x030000
# undef __ASSUME_PSELECT
# undef __ASSUME_PPOLL
#endif

/* No support for PI futexes or robust mutexes before 3.10 for m68k.  */
#if __LINUX_KERNEL_VERSION < 0x030a00
# undef __ASSUME_FUTEX_LOCK_PI
# undef __ASSUME_REQUEUE_PI
# undef __ASSUME_SET_ROBUST_LIST
#endif
