/* Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
   Contributed by Richard Henderson (rth@tamu.edu).

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

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
	save	%sp,-96,%sp

	/* sanity check arguments */
	tst	%i0
	be	.Lerror
	 tst	%i1
	be	.Lerror
	 nop

	/* Do the system call */
	mov	%i1,%o1
	mov	%i2,%o0
	set	__NR_clone,%g1
	ta	0x10
	bcs	.Lerror
	 tst	%o1
	bne	__thread_start
	 nop
	mov	%o0,%i0
	ret
	 restore

.Lerror:
	call	__errno_location
	 or	%g0,EINVAL,%i0
	st	%i0,[%o0]
	mov	-1,%i0
	ret
	 restore

	.size	__clone, .-__clone

	.type	__thread_start,@function

__thread_start:
	call	%i0
	 mov	%i3,%o0
	call	_exit,0
	 nop

	.size	__thread_start, .-__thread_start

weak_alias(__clone, clone)
