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
#include <kernel-features.h>

#ifdef UP
# define LOCK
#else
# define LOCK lock
#endif

#define SYS_futex		202
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1
#define FUTEX_REQUEUE		3

#define EINVAL			22


	.text

	/* int pthread_cond_signal (pthread_cond_t *cond) */
	.globl	__pthread_cond_signal
	.type	__pthread_cond_signal, @function
	.align	16
__pthread_cond_signal:

	/* Get internal lock.  */
	movq	%rdi, %r8
	movl	$1, %esi
	LOCK
#if cond_lock == 0
	xaddl	%esi, (%rdi)
#else
	xaddl	%esi, cond_lock(%rdi)
#endif
	testl	%esi, %esi
	jne	1f

2:	addq	$wakeup_seq, %rdi
	movq	total_seq(%r8), %rcx
	cmpq	(%rdi), %rcx
	jbe	4f

	/* Bump the wakeup number.  */
	addq	$1, (%rdi)

	/* Wake up one thread.  */
	movq	$FUTEX_REQUEUE, %rsi
	movq	$SYS_futex, %rax
	xorq	%rdx, %rdx
	movq	$1, %r10
	syscall

#ifndef __ASSUME_FUTEX_REQUEUE
	cmpq	$-EINVAL, %rax
	je	7f
#endif

	/* If we moved a thread we in any case have to make the syscall.  */
	testq	%rax, %rax
	jne	5f

	/* Unlock.  */
4:	LOCK
#if cond_lock == 0
	decl	(%r8)
#else
	decl	cond_lock(%r8)
#endif
	jne	5f

6:	xorl	%eax, %eax
	retq

	/* Initial locking failed.  */
1:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	callq	__lll_mutex_lock_wait
#if cond_lock != 0
	subq	$cond_lock, %rdi
#endif
	jmp	2b

	/* Unlock in loop requires wakeup.  */
5:
#if cond_lock != 0
	addq	$cond_lock-wakeup_seq, %rdi
#else
	movq	%r8, %rdi
#endif
	callq	__lll_mutex_unlock_wake
	jmp	6b

#ifndef __ASSUME_FUTEX_REQUEUE
7:	/* The futex requeue functionality is not available.  */
	movq	$1, %rdx
	movq	$FUTEX_WAKE, %rsi
	movq	$SYS_futex, %rax
	syscall
	jmp	4b
#endif
	.size	__pthread_cond_signal, .-__pthread_cond_signal
versioned_symbol (libpthread, __pthread_cond_signal, pthread_cond_signal,
		  GLIBC_2_3_2)
