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
#include <tcb-offsets.h>

#ifdef UP
# define LOCK
#else
# define LOCK lock
#endif

#define SYS_futex		202
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1


	.text

	.align	16
	.type	__condvar_cleanup, @function
	.globl	__condvar_cleanup
	.hidden	__condvar_cleanup
__condvar_cleanup:
	/* Get internal lock.  */
	movq	%rdi, %r8
	movl	8(%rdi), %rdi
	movl	$1, %esi
	LOCK
#if cond_lock == 0
	xaddl	%esi, (%rdi)
#else
	xaddl	%esi, cond_lock(%rdi)
#endif
	testl	%esi, %esi
	je	1f

#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	callq	__lll_mutex_lock_wait
#if cond_lock != 0
	subq	$cond_lock, %rdi
#endif

1:	addq	$1, wakeup_seq(%rdi)

	addq	$1, woken_seq(%rdi)

	/* Wake up all waiters to make sure no signal gets lost.  */
	addq	$wakeup_seq, %rdi
	movq	$FUTEX_WAKE, %rsi
	movl	$0x7fffffff, %edx
	movq	$SYS_futex, %rax
	syscall
	subq	$wakeup_seq, %rdi

	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	je	2f
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	callq	__lll_mutex_unlock_wake

	/* Lock the mutex unless asynchronous cancellation is in effect.  */
2:	testq	$2, (%r8)
	jne	3f

	movq	16(%r8), %edi
	callq	__pthread_mutex_lock_internal

3:	retq
	.size	__condvar_cleanup, .-__condvar_cleanup


/* int pthread_cond_wait (pthread_cond_t *cond, pthread_mutex_t *mutex)  */
	.globl	__pthread_cond_wait
	.type	__pthread_cond_wait, @function
	.align	16
__pthread_cond_wait:
	pushq	%12
	subq	$64, %rsp

	/* Prepare structure passed to cancellation handler.  */
	movq	%rdi, 8(%rsp)
	movq	%rsi, 16(%rsp)

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
	jne	12f

	movq	(%rsp), %rdi
	addq	$1, total_seq(%rdi)

	/* Install cancellation handler.  */
#ifdef PIC
	leaq	__condvar_cleanup@GOTOFF(%rip), %rsi
#else
	leaq	__condvar_cleanup, %rsi
#endif
	leaq	32(%rsp), %rdi
	movq	%rsp, %rdx
	callq	__pthread_cleanup_push

	/* Get and store current wakeup_seq value.  */
	movq	8(%rsp), %rdi
	movq	wakeup_seq(%rdi), %r12
	movq	%r12, 24(%rsp)

	/* Unlock.  */
8:	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	jne	3f

4:	callq	__pthread_enable_asynccancel
	movq	%rax, (%rsp)

	xorq	%r10, %r10
	movq	%r12, %rdx
	addq	$wakeup_seq-cond_lock, %rdi
	movq	$SYS_futex, %rax
	movq	%r10, %rsi	/* movl $FUTEX_WAIT, %ecx */
	syscall
	subl	$wakeup_seq-cond_lock, %ebx

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

	cmpq	24(%rsp), %rax
	jb	8b

	cmpq	%rax, %r12
	jna	8b

	incq	woken_seq(%rdi)

	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	jne	10f

	/* Remove cancellation handler.  */
11:	movq	32+CLEANUP_PREV(%rsp), %rdx
	movq	%rdx, %gs:CLEANUP

	movq	16(%rsp), %rdi
	callq	__pthread_mutex_lock_internal
14:	addq	$64, %rsp

	popq	%r12

	/* We return the result of the mutex_lock operation.  */
	retq

	/* Initial locking failed.  */
1:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	callq	__lll_mutex_lock_wait
	jmp	2b

	/* Unlock in loop requires waekup.  */
3:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	callq	__lll_mutex_unlock_wake
#if cond_lock != 0
	subq	$cond_lock, %rdi
#endif
	jmp	4b

	/* Locking in loop failed.  */
5:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	callq	__lll_mutex_lock_wait
#if cond_lock != 0
	subq	$cond_lock, %rdi
#endif
	jmp	6b

	/* Unlock after loop requires wakeup.  */
10:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	callq	__lll_mutex_unlock_wake
	jmp	11b

	/* The initial unlocking of the mutex failed.  */
12:	movq	%rax, %r10
	movq	8(%rsp), %rdi
	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	jne	13f

#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	callq	__lll_mutex_unlock_wake

13:	movq	%r10, %rax
	jmp	14b
	.size	__pthread_cond_wait, .-__pthread_cond_wait
versioned_symbol (libpthread, __pthread_cond_wait, pthread_cond_wait,
		  GLIBC_2_3_2)
