/* Copyright (C) 1997, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson (rth@tamu.edu).

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

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <asm/errno.h>
#include <asm/unistd.h>

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg); */

	.text
	.align	4
	.globl	__clone
	.type	__clone,@function

__clone:
	save	%sp, -192, %sp

	/* sanity check arguments */
	brz,pn	%i0, 99f
	 mov	%i0, %l0		/* save fn */
	brz,pn	%i1, 99f
	 mov	%i3, %l3		/* save arg */

	/* Do the system call */
	sub	%i1, 0x7ff, %o1
	mov	%i2, %o0
	set	__NR_clone, %g1
	ta	0x6d
	bcs,pn	%xcc, 99f
	 nop
	brnz,pn	%o1, __thread_start
	 mov	%o0, %i0
	ret
	 restore
99:
#ifndef _LIBC_REENTRANT
#ifdef PIC
	call	1f
	sethi	%hi(_GLOBAL_OFFSET_TABLE_-(99b-.)), %l7
1:	or	%l7, %lo(_GLOBAL_OFFSET_TABLE_-(99b-.)), %l7
	add	%l7, %o7, %l7
	set	EINVAL, %i0
	sethi	%hi(errno), %g2
	or	%g2, %lo(errno), %g2
	st	%i0, [%l7+%g2]
#else
	sethi	%hi(errno), %g2
	set	EINVAL, %i0
	st	%i0, [%g2+%lo(errno)]
#endif
#else
	call	__errno_location
	 nop
	st	%i0, [%o0]
#endif
	ret
	 restore %g0,-1,%o0
	.size __clone, .-__clone

	.type __thread_start,@function
__thread_start:
	mov	%g0, %fp	/* terminate backtrace */
	sub	%sp, 6*8, %sp	/* provide arg storage */
	call	%l0
	 mov	%l3,%o0
	call	_exit,0
	 nop
	.size __thread_start, .-__thread_start

weak_alias(__clone, clone)
