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
#include <shlib-compat.h>
#include <lowlevelcond.h>
#include <pthread-errnos.h>

#ifdef UP
# define LOCK
#else
# define LOCK lock
#endif

#define SYS_gettimeofday	__NR_gettimeofday
#define SYS_futex		202
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1


	.text

/* int pthread_cond_timedwait (pthread_cond_t *cond, pthread_mutex_t *mutex,
			       const struct timespec *abstime)  */
	.globl	__pthread_cond_timedwait
	.type	__pthread_cond_timedwait, @function
	.align	16
__pthread_cond_timedwait:
	pushq	%r12
	pushq	%r13
	pushq	%r14
	subq	$80, %rsp

	/* Prepare structure passed to cancellation handler.  */
	movq	%rdi, 8(%rsp)
	movq	%rsi, 16(%rsp)
	movq	%rdx, %r13

	/* Get internal lock.  */
	movl	$1, %esi
	LOCK
#if cond_lock == 0
	xaddl	%esi, (%rdi)
#else
	xaddl	%esi, cond_lock(%rdi)
#endif
	testl	%esi, %esi
	jne	1f

	/* Unlock the mutex.  */
2:	movq	16(%rsp), %rdi
	callq	__pthread_mutex_unlock_internal

	testl	%eax, %eax
	jne	16f

	movq	8(%rsp), %rdi
	addq	$1, total_seq(%rdi)

	/* Install cancellation handler.  */
#ifdef PIC
	leaq	__condvar_cleanup(%rip), %rsi
#else
	leaq	__condvar_cleanup, %rsi
#endif
	leaq	48(%rsp), %rdi
	movq	%rsp, %rdx
	callq	__pthread_cleanup_push

	/* Get and store current wakeup_seq value.  */
	movq	8(%rsp), %rdi
	movq	wakeup_seq(%rdi), %r12
	movq	%r12, 40(%rsp)

	/* Unlock.  */
8:	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	jne	3f

4:	movq	%rsp, %rdi
	call	__pthread_enable_asynccancel_2

	/* Get the current time.  */
	leaq	24(%rsp), %rdi
	xorq	%rcx, %rcx
	movq	$SYS_gettimeofday, %rax
	syscall

	/* Compute relative timeout.  */
	movq	32(%rsp), %rax
	movq	$1000, %rdx
	mul	%rdx		/* Milli seconds to nano seconds.  */
	movq	(%r13), %rcx
	movq	4(%r13), %rdx
	subq	24(%rsp), %rcx
	subq	%rax, %rdx
	jns	12f
	addq	$1000000000, %rdx
	decq	%rcx
12:	testq	%rcx, %rcx
	js	13f

	/* Store relative timeout.  */
	movq	%rcx, 24(%rsp)
	movq	%rdx, 32(%rsp)

	movq	8(%rsp), %rdi
	leaq	24(%rsp), %r10
	xorq	%rsi, %rsi	/* movq $FUTEX_WAIT, %rsi */
	movq	%r12, %rdx
	addq	$wakeup_seq-cond_lock, %rdi
	movq	$SYS_futex, %rax
	syscall
	movq	%rax, %r14

	movq	(%rsp), %rdi
	callq	__pthread_disable_asynccancel

	/* Lock.  */
	movq	8(%rsp), %rdi
	movl	$1, %esi
	LOCK
#if cond_lock == 0
	xaddl	%esi, (%rdi)
#else
	xaddl	%esi, cond_lock(%rdi)
#endif
	testl	%esi, %esi
	jne	5f

6:	movq	woken_seq(%rdi), %rax

	movq	wakeup_seq(%rdi), %r12

	cmpq	40(%rsp), %rax
	jb	15f

	cmpq	%rax, %r12
	ja	9f

15:	cmpq	$-ETIMEDOUT, %r14
	jne	8b

13:	incq	wakeup_seq(%rdi)
	movq	$ETIMEDOUT, %rsi
	jmp	14f

9:	xorq	%rsi, %rsi
14:	incq	woken_seq(%rdi)

	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	jne	10f

	/* Remove cancellation handler.  */
11:	movq	48+CLEANUP_PREV(%rsp), %rdx
	movq	%rdx, %fs:CLEANUP

	movq	16(%rsp), %rdi
	callq	__pthread_mutex_lock_internal

	testq	%rax, %rax
	cmoveq	%rsi, %rax

18:	addq	$80, %rsp
	popq	%r14
	popq	%r13
	popq	%r12

	retq

	/* Initial locking failed.  */
1:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	call	__lll_mutex_lock_wait
	jmp	2b

	/* Unlock in loop requires waekup.  */
3:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	call	__lll_mutex_unlock_wake
	jmp	4b

	/* Locking in loop failed.  */
5:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	call	__lll_mutex_lock_wait
#if cond_lock != 0
	subq	$cond_lock, %rdi
#endif
	jmp	6b

	/* Unlock after loop requires waekup.  */
10:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	call	__lll_mutex_unlock_wake
	jmp	11b

	/* The initial unlocking of the mutex failed.  */
16:	movq	8(%rsp), %rdi
	movq	%rax, (%rsp)
	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	jne	17f

#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	call	__lll_mutex_unlock_wake

17:	movq	(%rsp), %rax
	jmp	18b
	.size	__pthread_cond_wait, .-__pthread_cond_wait
	.size	__pthread_cond_timedwait, .-__pthread_cond_timedwait
versioned_symbol (libpthread, __pthread_cond_timedwait, pthread_cond_timedwait,
		  GLIBC_2_3_2)
