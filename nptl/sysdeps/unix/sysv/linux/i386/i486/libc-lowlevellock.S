/* Copyright (C) 2002, 2003, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <kernel-features.h>

/* All locks in libc are private.  Use the kernel feature if possible.  */
#define FUTEX_PRIVATE_FLAG	128
#ifdef __ASSUME_PRIVATE_FUTEX
# define FUTEX_WAIT		(0 | FUTEX_PRIVATE_FLAG)
# define FUTEX_WAKE		(1 | FUTEX_PRIVATE_FLAG)
#else
# define LOAD_FUTEX_WAIT(reg) \
	movl	%gs:PRIVATE_FUTEX, reg
# define LOAD_FUTEX_WAKE(reg) \
	movl	%gs:PRIVATE_FUTEX, reg ; \
	orl	$FUTEX_WAKE, reg
#endif

#include "lowlevellock.S"
