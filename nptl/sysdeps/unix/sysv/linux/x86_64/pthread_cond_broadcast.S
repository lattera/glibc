/* Copyright (C) 2002, 2003, 2004 Free Software Foundation, Inc.
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
#define FUTEX_CMP_REQUEUE	4

#define EINVAL			22


	.text

	/* int pthread_cond_broadcast (pthread_cond_t *cond) */
	.globl	__pthread_cond_broadcast
	.type	__pthread_cond_broadcast, @function
	.align	16
__pthread_cond_broadcast:

	/* Get internal lock.  */
	movl	$1, %esi
	xorl	%eax, %eax
	LOCK
#if cond_lock == 0
	cmpxchgl %esi, (%rdi)
#else
	cmpxchgl %esi, cond_lock(%rdi)
#endif
	jnz	1f

2:	addq	$cond_futex, %rdi
	movq	total_seq-cond_futex(%rdi), %r9
	cmpq	wakeup_seq-cond_futex(%rdi), %r9
	jna	4f

	/* Cause all currently waiting threads to recognize they are
	   woken up.  */
	movq	%r9, wakeup_seq-cond_futex(%rdi)
	movq	%r9, woken_seq-cond_futex(%rdi)
	addq	%r9, %r9
	movl	%r9d, (%rdi)
	incl	broadcast_seq-cond_futex(%rdi)

	/* Get the address of the mutex used.  */
	movq	dep_mutex-cond_futex(%rdi), %r8

	/* Unlock.  */
	LOCK
	decl	cond_lock-cond_futex(%rdi)
	jne	7f

8:	cmpq	$-1, %r8
	je	9f

	/* Wake up all threads.  */
	movq	$FUTEX_CMP_REQUEUE, %rsi
	movq	$SYS_futex, %rax
	movl	$1, %edx
	movq	$0x7fffffff, %r10
	syscall

	/* For any kind of error, which mainly is EAGAIN, we try again
	   with WAKE.  The general test also covers running on old
	   kernels.  */
	cmpq	$-4095, %rax
	jae	9f

10:	xorl	%eax, %eax
	retq

	.align	16
	/* Unlock.  */
4:	LOCK
	decl	cond_lock-cond_futex(%rdi)
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
5:	addq	$cond_lock-cond_futex, %rdi
	callq	__lll_mutex_unlock_wake
	jmp	6b

	/* Unlock in loop requires wakeup.  */
7:	addq	$cond_lock-cond_futex, %rdi
	callq	__lll_mutex_unlock_wake
	subq	$cond_lock-cond_futex, %rdi
	jmp	8b

9:	/* The futex requeue functionality is not available.  */
	movq	$0x7fffffff, %rdx
	movq	$FUTEX_WAKE, %rsi
	movq	$SYS_futex, %rax
	syscall
	jmp	10b
	.size	__pthread_cond_broadcast, .-__pthread_cond_broadcast
versioned_symbol (libpthread, __pthread_cond_broadcast, pthread_cond_broadcast,
		  GLIBC_2_3_2)
