/* Copyright (C) 2002, 2003, 2004, 2005 Free Software Foundation, Inc.
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

#define SYS_futex		202
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1

/* For the calculation see asm/vsyscall.h.  */
#define VSYSCALL_ADDR_vgettimeofday	0xffffffffff600000


	.globl	__lll_mutex_lock_wait
	.type	__lll_mutex_lock_wait,@function
	.hidden	__lll_mutex_lock_wait
	.align	16
__lll_mutex_lock_wait:
	pushq	%r10
	pushq	%rdx

	xorq	%r10, %r10	/* No timeout.  */
	movl	$2, %edx
#if FUTEX_WAIT == 0
	xorl	%esi, %esi
#else
	movl	$FUTEX_WAIT, %esi
#endif

	cmpl	%edx, %eax	/* NB:	 %edx == 2 */
	jne	2f

1:	movl	$SYS_futex, %eax
	syscall

2:	movl	%edx, %eax
	xchgl	%eax, (%rdi)	/* NB:	 lock is implied */

	testl	%eax, %eax
	jnz	1b

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
	cmpq	$1000000000, 8(%rdx)
	jae	3f

	pushq	%r8
	pushq	%r9
	pushq	%r12
	pushq	%r13
	pushq	%r14

	/* Stack frame for the timespec and timeval structs.  */
	subq	$16, %rsp

	movq	%rdi, %r12
	movq	%rdx, %r13

1:
	/* Get current time.  */
	movq	%rsp, %rdi
	xorl	%esi, %esi
	movq	$VSYSCALL_ADDR_vgettimeofday, %rax
	/* This is a regular function call, all caller-save registers
	   might be clobbered.  */
	callq	*%rax

	/* Compute relative timeout.  */
	movq	8(%rsp), %rax
	movl	$1000, %edi
	mul	%rdi		/* Milli seconds to nano seconds.  */
	movq	(%r13), %rdi
	movq	8(%r13), %rsi
	subq	(%rsp), %rdi
	subq	%rax, %rsi
	jns	4f
	addq	$1000000000, %rsi
	decq	%rdi
4:	testq	%rdi, %rdi
	js	5f		/* Time is already up.  */

	/* Futex call.  */
	movq	%rdi, (%rsp)	/* Store relative timeout.  */
	movq	%rsi, 8(%rsp)

	movl	$1, %eax
	movl	$2, %edx
	LOCK
	cmpxchgl %edx, (%r12)

	testl	%eax, %eax
	je	8f

	movq	%rsp, %r10
#if FUTEX_WAIT == 0
	xorl	%esi, %esi
#else
	movl	$FUTEX_WAIT, %esi
#endif
	movq	%r12, %rdi
	movl	$SYS_futex, %eax
	syscall
	movq	%rax, %rcx

8:				/* NB: %edx == 2 */
	xorl	%eax, %eax
	LOCK
	cmpxchgl %edx, (%rdi)
	jnz	7f

6:	addq	$16, %rsp
	popq	%r14
	popq	%r13
	popq	%r12
	popq	%r9
	popq	%r8
	retq

	/* Check whether the time expired.  */
7:	cmpq	$-ETIMEDOUT, %rcx
	je	5f

	/* Make sure the current holder knows we are going to sleep.  */
	movl	%edx, %eax
	xchgl	%eax, (%rdi)
	testl	%eax, %eax
	jz	6b
	jmp	1b

3:	movl	$EINVAL, %eax
	retq

5:	movl	$ETIMEDOUT, %eax
	jmp	6b
	.size	__lll_mutex_timedlock_wait,.-__lll_mutex_timedlock_wait
#endif


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


	.globl	__lll_mutex_unlock_wake
	.type	__lll_mutex_unlock_wake,@function
	.hidden	__lll_mutex_unlock_wake
	.align	16
__lll_mutex_unlock_wake:
	pushq	%rsi
	pushq	%rdx

	movl	$0, (%rdi)
	movl	$FUTEX_WAKE, %esi
	movl	$1, %edx	/* Wake one thread.  */
	movl	$SYS_futex, %eax
	syscall

	popq	%rdx
	popq	%rsi
	retq
	.size	__lll_mutex_unlock_wake,.-__lll_mutex_unlock_wake


#ifdef NOT_IN_libc
	.globl	__lll_timedwait_tid
	.type	__lll_timedwait_tid,@function
	.hidden	__lll_timedwait_tid
	.align	16
__lll_timedwait_tid:
	pushq	%r12
	pushq	%r13

	movq	%rdi, %r12
	movq	%rsi, %r13

	subq	$16, %rsp

	/* Get current time.  */
2:	movq	%rsp, %rdi
	xorl	%esi, %esi
	movq	$VSYSCALL_ADDR_vgettimeofday, %rax
	callq	*%rax

	/* Compute relative timeout.  */
	movq	8(%rsp), %rax
	movl	$1000, %edi
	mul	%rdi		/* Milli seconds to nano seconds.  */
	movq	(%r13), %rdi
	movq	8(%r13), %rsi
	subq	(%rsp), %rdi
	subq	%rax, %rsi
	jns	5f
	addq	$1000000000, %rsi
	decq	%rdi
5:	testq	%rdi, %rdi
	js	6f		/* Time is already up.  */

	movq	%rdi, (%rsp)	/* Store relative timeout.  */
	movq	%rsi, 8(%rsp)

	movl	(%r12), %edx
	testl	%edx, %edx
	jz	4f

	movq	%rsp, %r10
#if FUTEX_WAIT == 0
	xorl	%esi, %esi
#else
	movl	$FUTEX_WAIT, %esi
#endif
	movq	%r12, %rdi
	movl	$SYS_futex, %eax
	syscall

	cmpl	$0, (%rdi)
	jne	1f
4:	xorl	%eax, %eax

8:	addq	$16, %rsp
	popq	%r13
	popq	%r12
	retq

1:	cmpq	$-ETIMEDOUT, %rax
	jne	2b

6:	movl	$ETIMEDOUT, %eax
	jmp	8b
	.size	__lll_timedwait_tid,.-__lll_timedwait_tid
#endif
