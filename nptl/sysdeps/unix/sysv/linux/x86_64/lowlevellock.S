/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
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

#include <sysdep.h>

	.text

#ifndef LOCK
# ifdef UP
#  define LOCK
# else
#  define LOCK lock
# endif
#endif

#define SYS_gettimeofday	__NR_gettimeofday
#define SYS_futex		202
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1

#define ETIMEDOUT               110


	/* Modified: %rax, %rsi.  */
	.globl	__lll_lock_wait
	.type	__lll_lock_wait,@function
	.hidden	__lll_lock_wait
	.align	16
__lll_lock_wait:
	pushq	%r10
	pushq	%rdx

	xorq	%r10, %r10	/* No timeout.  */

1:
	leal	-1(%esi), %edx	/* account for the preceeded xadd.  */
	movq	%r10, %rsi	/* movl $FUTEX_WAIT, %ecx */
	movq	$SYS_futex, %rax
	syscall

	orl	$-1, %esi	/* Load -1.  */
	LOCK
	xaddl	%esi, (%rdi)
	jne,pn	1b

	movl	$-1, (%rdi)

	popq	%rdx
	popq	%r10
	retq
	.size	__lll_lock_wait,.-__lll_lock_wait


#ifdef NOT_IN_libc
	.globl	lll_unlock_wake_cb
	.type	lll_unlock_wake_cb,@function
	.hidden	lll_unlock_wake_cb
	.align	16
lll_unlock_wake_cb:
	pushq	%rsi
	pushq	%rdx

	LOCK
	addl	$1, (%rdi)
	jng	1f

	popq	%rdx
	popq	%rsi
	retq
	.size	lll_unlock_wake_cb,.-lll_unlock_wake_cb
#endif


	.globl	__lll_unlock_wake
	.type	__lll_unlock_wake,@function
	.hidden	__lll_unlock_wake
	.align	16
__lll_unlock_wake:
	pushq	%rsi
	pushq	%rdx

1:	movq	$FUTEX_WAKE, %rsi
	movl	$1, %edx	/* Wake one thread.  */
	movq	$SYS_futex, %rax
	movl	%edx, (%rdi)	/* Stores '$1'.  */
	syscall

	popq	%rdx
	popq	%rsi
	retq
	.size	__lll_unlock_wake,.-__lll_unlock_wake


#ifdef NOT_IN_libc
	.globl	__lll_timedwait_tid
	.type	__lll_timedwait_tid,@function
	.hidden	__lll_timedwait_tid
	.align	16
__lll_timedwait_tid:
	movl	%rdi, %r8
	movl	%rsi, %r9

	subq	$16, %rsp

	/* Get current time.  */
2:	movq	%rsp, %rdi
	xorq	%rsi, %rsi
	movq	$SYS_gettimeofday, %rax
	syscall

	/* Compute relative timeout.  */
	movq	8(%esp), %rax
	movq	$1000, %rdi
	mul	%rdx		/* Milli seconds to nano seconds.  */
	movq	(%r9), %rdi
	movq	8(%r9), %rsi
	subq	(%rsp), %rdi
	subq	%rax, %rsi
	jns	5f
	addq	$1000000000, %rsi
	decq	%rdi
5:	testq	%rdi, %rdi
	js	6f		/* Time is already up.  */

	movq	%rdi, (%rsp)	/* Store relative timeout.  */
	movq	%rsi, 8(%rsp)

	movl	(%r8), %edx
	testl	%edx, %edx
	jz	4f

	movq	%rsp, %r10
	xorq	%rsi, %rsi	/* movl $FUTEX_WAIT, %ecx */
	movq	%r8, %rdi
	movq	$SYS_futex, %rax
	syscall

	cmpl	$0, (%rdi)
	jne	1f
4:	xorl	%eax, %eax

	addq	$16, %rsp
	retq

1:	cmpq	$-ETIMEDOUT, %rax
	jne	2b

6:	movl	$ETIMEDOUT, %eax
	addq	$16, %rsp
	retq
	.size	__lll_timedwait_tid,.-__lll_timedwait_tid
#endif
