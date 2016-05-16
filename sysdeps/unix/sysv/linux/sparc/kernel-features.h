/* Set flags signalling availability of kernel features based on given
   kernel version number.  SPARC version.
   Copyright (C) 1999-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* SPARC uses socketcall.  */
#define __ASSUME_SOCKETCALL		1

/* The accept4 syscall was added for SPARC in 2.6.28.  */
#define __ASSUME_ACCEPT4_SYSCALL_WITH_SOCKETCALL	1

/* The recvmmsg syscall was added for SPARC in 2.6.33.  */
#define __ASSUME_RECVMMSG_SYSCALL_WITH_SOCKETCALL	1

/* The sendmmsg syscall was added for SPARC in 3.0.  */
#define __ASSUME_SENDMMSG_SYSCALL_WITH_SOCKETCALL	1

#include_next <kernel-features.h>

/* 32-bit SPARC kernels do not support
   futex_atomic_cmpxchg_inatomic.  */
#if !defined __arch64__ && !defined __sparc_v9__
# undef __ASSUME_REQUEUE_PI
# undef __ASSUME_SET_ROBUST_LIST
#endif
