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
#include <lowlevelrwlock.h>
#include <pthread-errnos.h>


#define SYS_gettimeofday	__NR_gettimeofday
#define SYS_futex		202
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1

#ifndef UP
# define LOCK lock
#else
# define LOCK
#endif


	.text

	.globl	pthread_rwlock_timedrdlock
	.type	pthread_rwlock_timedrdlock,@function
	.align	16
pthread_rwlock_timedrdlock:
	subq	$16, %rsp

	movq	%rdi, %r8
	movq	%rsi, %r9
	xorq	%r10, %r10

	/* Get the lock.  */
	movl	$1, %esi
	LOCK
#if MUTEX == 0
	xaddl	%esi, (%rdi)
#else
	xaddl	%esi, MUTEX(%rdi)
#endif
	testl	%esi, %esi
	jne	1f

2:	movq	WRITER(%r8), %rax
	testq	%rax, %rax
	jne	14f
	cmpl	$0, WRITERS_QUEUED(%r8)
	je	5f
	cmpl	$0, FLAGS(%r8)
	je	5f

	/* Check the value of the timeout parameter.  */
3:	cmpl	$1000000000, 8(%r9)
	jae	19f

	incl	READERS_QUEUED(%r8)
	je	4f

	movl	READERS_WAKEUP(%r8), %edx

	/* Unlock.  */
	LOCK
#if MUTEX == 0
	subl	$1, (%r8)
#else
	subl	$1, MUTEX(%r8)
#endif
	jne	10f

	/* Get current time.  */
11:	movq	%rsp, %rdi
	xorq	%rsi, %rsi
	movq	$SYS_gettimeofday, %rax
	syscall

	/* Compute relative timeout.  */
	movq	8(%rsp), %rax
	movq	$1000, %rdi
	mul	%rdi		/* Milli seconds to nano seconds.  */
	movq	(%r9), %rcx
	movq	8(%r9), %rdi
	subq	(%rsp), %rcx
	subq	%rax, %rdi
	jns	15f
	addq	$1000000000, %rdi
	decq	%rcx
15:	testq	%rcx, %rcx
	js	16f		/* Time is already up.  */

	/* Futex call.  */
	movq	%rcx, (%rsp)	/* Store relative timeout.  */
	movq	%rdi, 8(%rsp)

	xorq	%rsi, %rsi	/* movq $FUTEX_WAIT, %rsi */
	leaq	READERS_WAKEUP(%r8), %rdi
	movq	$SYS_futex, %rax
	syscall
	movq	%rax, %rdx
17:

	/* Reget the lock.  */
	movl	$1, %esi
	LOCK
#if MUTEX == 0
	xaddl	%esi, (%r8)
#else
	xaddl	%esi, MUTEX(%r8)
#endif
	testl	%esi, %esi
	jne	12f

13:	decl	READERS_QUEUED(%r8)
	cmpq	$-ETIMEDOUT, %rdx
	jne	2b

18:	movq	$ETIMEDOUT, %rcx
	jmp	9f


5:	xorq	%rcx, %rcx
	incl	NR_READERS(%r8)
	je	8f
9:	LOCK
#if MUTEX == 0
	decl	(%r8)
#else
	decl	MUTEX(%r8)
#endif
	jne	6f

7:	movq	%rcx, %rax

	addq	$16, %rsp
	retq

1:
#if MUTEX != 0
	addq	$MUTEX, %rdi
#endif
	callq	__lll_mutex_lock_wait
	jmp	2b

14:	cmpq	%fs:SELF, %rax
	jne	3b
	movq	$EDEADLK, %rcx
	jmp	9b

6:
#if MUTEX == 0
	movq	%r8, %rdi
#else
	leal	MUTEX(%r8), %rdi
#endif
	callq	__lll_mutex_unlock_wake
	jmp	7b

	/* Overflow.  */
8:	decl	NR_READERS(%r8)
	movq	$EAGAIN, %rcx
	jmp	9b

	/* Overflow.  */
4:	decl	READERS_QUEUED(%r8)
	movq	$EAGAIN, %rcx
	jmp	9b

10:
#if MUTEX == 0
	movq	%r8, %rdi
#else
	leaq	MUTEX(%r8), %rdi
#endif
	callq	__lll_mutex_unlock_wake
	jmp	11b

12:
#if MUTEX == 0
	movq	%r8, %rdi
#else
	leaq	MUTEX(%r8), %rdi
#endif
	call	__lll_mutex_lock_wait
	jmp	13b

16:	movq	$-ETIMEDOUT, %rdx
	jmp	17b

19:	movq	$EINVAL, %rcx
	jmp	9b
	.size	pthread_rwlock_timedrdlock,.-pthread_rwlock_timedrdlock
