/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

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

#include <sysdep-cancel.h>

	.text
ENTRY(__vfork)
	ld	[%g6 + MULTIPLE_THREADS_OFFSET], %o0
	cmp	%o0, 0
	bne	HIDDEN_JUMPTARGET(__fork)
	 mov	__NR_vfork, %g1
	ta 0x10;
	bcs	__syscall_error_handler
	 nop
	sub	%o1, 1, %o1
	retl
	 and	%o0, %o1, %o0
	SYSCALL_ERROR_HANDLER
PSEUDO_END (__vfork)
libc_hidden_def (__vfork)
weak_alias (__vfork, vfork)
