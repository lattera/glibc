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
#include <pthread-errnos.h>

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


	.globl	__lll_mutex_lock_wait
	.type	__lll_mutex_lock_wait,@function
	.hidden	__lll_mutex_lock_wait
	.align	16
__lll_mutex_lock_wait:
	pushq	%r10
	pushq	%rdx

	xorq	%r10, %r10	/* No timeout.  */

1:
	leaq	1(%rsi), %rdx	/* account for the preceeded xadd.  */
	movq	%r10, %rsi	/* movl $FUTEX_WAIT, %ecx */
	movq	$SYS_futex, %rax
	syscall

	movl	$1, %esi
	LOCK
	xaddl	%esi, (%rdi)
	testl	%esi, %esi
	jne	1b

	movl	$2, (%rdi)

	popq	%rdx
	popq	%r10
	retq
	.size	__lll_mutex_lock_wait,.-__lll_mutex_lock_wait


#ifdef NOT_IN_libc
	.globl	__lll_mutex_timedlock_wait
	.type	__lll_mutex_timedlock_wait,@function
	.hidden	__lll_mutex_timedlock_wait
	.align	16
__lll_mutex_timedlock_wait:
	/* Check for a valid timeout value.  */
	cmpq	$1000000000, 4(%rdx)
	jae	3f

	/* Stack frame for the timespec and timeval structs.  */
	subq	$16, %rsp

	movq	%rdi, %r8
	movq	%rdx, %r9
	leaq	1(%rax), %rdx

	/* Get current time.  */
1:
	movq	%rsp, %rdi
	xorq	%rsi, %rsi
	movq	$SYS_gettimeofday, %rax
	syscall

	/* Compute relative timeout.  */
	movq	8(%rsp), %rax
	movq	$1000, %rdi
	mul	%rdi		/* Milli seconds to nano seconds.  */
	movq	(%r9), %rdi
	movq	8(%r9), %rsi
	subq	(%rsp), %rdi
	subq	%rax, %rsi
	jns	4f
	addq	$1000000000, %rsi
	decq	%rdi
4:	testq	%rdi, %rdi
	js	5f		/* Time is already up.  */

	/* Futex call.  */
	movq	%rdi, (%rsp)	/* Store relative timeout.  */
	movq	%rsi, 4(%rsp)

	movq	%rsp, %r10
	xorq	%rsi, %rsi	/* movq $FUTEX_WAIT, %rsi */
	movq	%r8, %rdi
	movq	$SYS_futex, %rax
	syscall

	movl	$1, %edx
	LOCK
	xaddl	%edx, (%rdi)
	testl	%edx, %edx
	jne	7f

	movl	$2, (%rdi)
	xorl	%eax, %eax

	addq	$16, %rsp
	retq

	/* Check whether the time expired.  */
7:	cmpq	$-ETIMEDOUT, %rax
	je	5f
	jmp	1b

3:	movl	$EINVAL, %eax
	retq

5:	movl	$ETIMEDOUT, %eax
	addq	$16, %rsp
	retq
	.size	__lll_mutex_timedlock_wait,.-__lll_mutex_timedlock_wait
#endif


	.globl	__lll_mutex_unlock_wake
	.type	__lll_mutex_unlock_wake,@function
	.hidden	__lll_mutex_unlock_wake
	.align	16
__lll_mutex_unlock_wake:
	pushq	%rsi
	pushq	%rdx

	movl	$0, (%rdi)
	movq	$FUTEX_WAKE, %rsi
	movl	$1, %edx	/* Wake one thread.  */
	movq	$SYS_futex, %rax
	syscall

	popq	%rdx
	popq	%rsi
	retq
	.size	__lll_mutex_unlock_wake,.-__lll_mutex_unlock_wake
