/* Copyright (C) 2011-2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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


/* TILE glibc support starts with 2.6.36, guaranteeing many kernel features. */
#define __ASSUME_UTIMES			1
#define __ASSUME_O_CLOEXEC		1
#define __ASSUME_SOCK_CLOEXEC		1
#define __ASSUME_IN_NONBLOCK		1
#define __ASSUME_PIPE2			1
#define __ASSUME_EVENTFD2		1
#define __ASSUME_SIGNALFD4		1
#define __ASSUME_ACCEPT4		1
#define __ASSUME_DUP3			1

#include_next <kernel-features.h>

/* Define this if your 32-bit syscall API requires 64-bit register
   pairs to start with an even-number register.  */
#define __ASSUME_ALIGNED_REGISTER_PAIRS	1
