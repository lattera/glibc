/* Copyright (C) 2002, 2003, 2004, 2005, 2006 Free Software Foundation, Inc.
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
#include <lowlevelrobustlock.h>

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
#define FUTEX_WAITERS		0x80000000
#define FUTEX_OWNER_DIED	0x40000000

/* For the calculation see asm/vsyscall.h.  */
#define VSYSCALL_ADDR_vgettimeofday	0xffffffffff600000


	.globl	__lll_robust_mutex_lock_wait
	.type	__lll_robust_mutex_lock_wait,@function
	.hidden	__lll_robust_mutex_lock_wait
	.align	16
__lll_robust_mutex_lock_wait:
	pushq	%r10
	pushq	%rdx

	xorq	%r10, %r10	/* No timeout.  */
#if FUTEX_WAIT == 0
	xorl	%esi, %esi
#else
	movl	$FUTEX_WAIT, %esi
#endif

4:	movl	%eax, %edx
	orl	$FUTEX_WAITERS, %edx

	testl	$FUTEX_OWNER_DIED, %eax
	jnz	3f

	cmpl	%edx, %eax
	je	1f

	LOCK
	cmpxchgl %edx, (%rdi)
	jnz	2f

1:	movl	$SYS_futex, %eax
	syscall

	movl	(%rdi), %eax

2:	testl	%eax, %eax
	jne	4b

	movl	%fs:TID, %edx
	LOCK
	cmpxchgl %edx, (%rdi)
	jnz	4b
	/* NB:	 %rax == 0 */

3:	popq	%rdx
	popq	%r10
	retq
	.size	__lll_robust_mutex_lock_wait,.-__lll_robust_mutex_lock_wait


	.globl	__lll_robust_mutex_timedlock_wait
	.type	__lll_robust_mutex_timedlock_wait,@function
	.hidden	__lll_robust_mutex_timedlock_wait
	.align	16
__lll_robust_mutex_timedlock_wait:
	/* Check for a valid timeout value.  */
	cmpq	$1000000000, 8(%rdx)
	jae	3f

	pushq	%r8
	pushq	%r9
	pushq	%r12
	pushq	%r13

	/* Stack frame for the timespec and timeval structs.  */
	subq	$24, %rsp

	movq	%rdi, %r12
	movq	%rdx, %r13

1:	movq	%rax, 16(%rsp)

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
	js	8f		/* Time is already up.  */

	/* Futex call.  */
	movq	%rdi, (%rsp)	/* Store relative timeout.  */
	movq	%rsi, 8(%rsp)

	movq	16(%rsp), %rdx
	movl	%edx, %eax
	orl	$FUTEX_WAITERS, %edx

	testl	$FUTEX_OWNER_DIED, %eax
	jnz	6f

	cmpl	%eax, %edx
	je	2f

	LOCK
	cmpxchgl %edx, (%r12)
	movq	$0, %rcx	/* Must use mov to avoid changing cc.  */
	jnz	5f

2:	movq	%rsp, %r10
#if FUTEX_WAIT == 0
	xorl	%esi, %esi
#else
	movl	$FUTEX_WAIT, %esi
#endif
	movq	%r12, %rdi
	movl	$SYS_futex, %eax
	syscall
	movq	%rax, %rcx

	movl	(%r12), %eax

5:	testl	%eax, %eax
	jne	7f

	movl	%fs:TID, %edx
	LOCK
	cmpxchgl %edx, (%r12)
	jnz	7f

6:	addq	$24, %rsp
	popq	%r13
	popq	%r12
	popq	%r9
	popq	%r8
	retq

	/* Check whether the time expired.  */
7:	cmpq	$-ETIMEDOUT, %rcx
	jne	1b

8:	movl	$ETIMEDOUT, %eax
	jmp	6b

3:	movl	$EINVAL, %eax
	retq
	.size	__lll_robust_mutex_timedlock_wait,.-__lll_robust_mutex_timedlock_wait
