/* Copyright (C) 1996, 1997, 1998, 2000, 2003, 2004
   Free Software Foundation, Inc.
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
#include <tcb-offsets.h>

#define CLONE_VM	0x00000100
#define CLONE_THREAD	0x00010000

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg,
	     pid_t *ptid, void *tls, pid_t *ctid); */

	.text
	.align	4
	.globl	__clone
	.type	__clone,@function

__clone:
	save	%sp,-96,%sp

	/* sanity check arguments */
	tst	%i0
	be	.Lerror
	 orcc	%i1,%g0,%o1
	be	.Lerror
	 mov	%i2,%o0
	/* ptid */
	mov	%i4,%o2
	/* tls */
	mov	%i5,%o3
	/* ctid */
	ld	[%fp+92],%o4

	/* Do the system call */
	set	__NR_clone,%g1
	ta	0x10
	bcs	.Lerror
	 tst	%o1
	bne	__thread_start
	 nop
	ret
	 restore %o0,%g0,%o0

.Lerror:
	call	__errno_location
	 or	%g0,EINVAL,%i0
	st	%i0,[%o0]
	ret
	 restore %g0,-1,%o0

	.size	__clone, .-__clone

	.type	__thread_start,@function

__thread_start:
#ifdef RESET_PID
	sethi	%hi(CLONE_THREAD), %l0
	andcc	%i2, %l0, %g0
	bne	1f
	 andcc	%i2, CLONE_VM, %g0
	bne,a	2f
	 mov	-1,%o0
	set	__NR_getpid,%g1
	ta	0x10
2:	st	%o0,[%g7 + PID]
	st	%o0,[%g7 + TID]
1:
#endif
	call	%i0
	 mov	%i3,%o0
	call	_exit,0
	 nop

	.size	__thread_start, .-__thread_start

weak_alias(__clone, clone)
