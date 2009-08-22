/* Copyright (C) 2002, 2003, 2005, 2007, 2009 Free Software Foundation, Inc.
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
#include <lowlevellock.h>
#include <lowlevelrwlock.h>
#include <pthread-errnos.h>
#include <kernel-features.h>


	.text

	.globl	__pthread_rwlock_rdlock
	.type	__pthread_rwlock_rdlock,@function
	.align	16
__pthread_rwlock_rdlock:
	cfi_startproc
	movq	$NRW_RC, %rax
	LOCK
	xaddq	%rax, NRW_WORD(%rdi)
	js	.Lundo

.Lagain1:
	movq	$(NRW_WL|NRW_WP), %rcx
	movq	$(NRW_WL|NRW_WW_MASK), %rdx
	testq	%rax, %rcx
	setnz	%cl
	testq	%rax, %rdx
	setnz	%dl
	orb	%cl, %dl
	jnz	.Lwait

	xorl	%eax, %eax
	ret

.Lwait:	movq	$NRW_WL, %rcx
	testq	%rcx, %rax
	jz	4f

	movl	%fs:TID, %ecx
	cmpl	%ecx, WRITER(%rdi)
	je	.Ldeadlk

4:	xorq	%r8, %r8
	movq	$NRW_RC, %rdx
	addq	%rdx, %rax
	movq	$(NRW_WL|NRW_AR), %rcx
	movq	$(NRW_WP|NRW_WW_MASK), %rdx
	testq	%rax, %rcx
	setz	%cl
	testq	%rax, %rdx
	setnz	%dl
	testb	%dl, %cl
	jz	1f

	movq	$NRW_RW-NRW_RC, %rdx
	addq	%rax, %rdx
	LOCK
	cmpxchgq %rdx, NRW_WORD(%rdi)
	jnz	.Lagain1

	movq	$NRW_RC_MASK, %rax
	testq	%rdx, %rax
	movl	$1, %r8d
	jz	.Lwake_waiter

1:	xorq	%r10, %r10
	movl	$NRW_R_WAKEUP, %r9d
	movl	$(FUTEX_WAIT_BITSET|FUTEX_PRIVATE_FLAG), %esi
	xorl	PSHARED(%rdi), %esi
	leaq	NRW_WORD+4(%rdi), %rdi
2:	shldq	$32, %rax, %rdx
	movl	$__NR_futex, %eax
	syscall
	movq	-4(%rdi), %rax

5:	movq	$(NRW_WL|NRW_WP), %rcx
	movq	$(NRW_WL|NRW_WW_MASK), %rdx
	testq	%rax, %rcx
	setnz	%cl
	testq	%rax, %rdx
	setnz	%dl
	orb	%cl, %dl
	jnz	2b

	testl	%r8d, %r8d
	jz	3f

	movq	$NRW_RC-NRW_RW, %rcx
	addq	%rcx, %rdx
	LOCK
	cmpxchgq %rdx, -4(%rdi)
	jnz	5b

3:	xorl	%eax, %eax
	ret

.Lwake_waiter:
	movq	%rdx, %r10
	movl	$__NR_futex, %eax
	movl	$(FUTEX_WAKE_BITSET|FUTEX_PRIVATE_FLAG), %esi
	xorl	PSHARED(%rdi), %esi
	leaq	NRW_WORD(%rdi), %rdi
	movl	$1, %edx
	movl	$NRW_W_WAKEUP, %r9d
	syscall
	leaq	-NRW_WORD(%rdi), %rdi
	movq	%r10, %rdx
	jmp	1b

.Lundo:	movq	$-NRW_RC, %rcx
	movq	%rax, %rdx
	subq	%rcx, %rax
	LOCK
	cmpxchgq %rdx, NRW_WORD(%rdi)
	jz	.Lret

	movq	$NRW_RC_OVFL, %r8
.Lagain2:
	testq	%r8, %rax
	jz	.Lagain1
	leaq	(%rax,%rcx), %rdx
	LOCK
	cmpxchgq %rdx, NRW_WORD(%rdi)
	jnz	.Lagain2

.Lret:	movl	$EAGAIN, %eax
	ret

.Ldeadlk:
	movq	$NRW_RC, %rdx
	LOCK
	subq	%rdx, NRW_WORD(%rdi)

	movl	$EDEADLK, %eax
	ret

#if 0
	xorq	%r10, %r10

	/* Get the lock.  */
	movl	$1, %esi
	xorl	%eax, %eax
	LOCK
#if MUTEX == 0
	cmpxchgl %esi, (%rdi)
#else
	cmpxchgl %esi, MUTEX(%rdi)
#endif
	jnz	1f

2:	movl	WRITER(%rdi), %eax
	testl	%eax, %eax
	jne	14f
	cmpl	$0, WRITERS_QUEUED(%rdi)
	je	5f
	cmpl	$0, FLAGS(%rdi)
	je	5f

3:	incl	READERS_QUEUED(%rdi)
	je	4f

	movl	READERS_WAKEUP(%rdi), %edx

	LOCK
#if MUTEX == 0
	decl	(%rdi)
#else
	decl	MUTEX(%rdi)
#endif
	jne	10f

11:
#ifdef __ASSUME_PRIVATE_FUTEX
	movl	$FUTEX_PRIVATE_FLAG|FUTEX_WAIT, %esi
	xorl	PSHARED(%rdi), %esi
#else
# if FUTEX_WAIT == 0
	movl	PSHARED(%rdi), %esi
# else
	movl	$FUTEX_WAIT, %esi
	orl	PSHARED(%rdi), %esi
# endif
	xorl	%fs:PRIVATE_FUTEX, %esi
#endif
	addq	$READERS_WAKEUP, %rdi
	movl	$SYS_futex, %eax
	syscall

	subq	$READERS_WAKEUP, %rdi

	/* Reget the lock.  */
	movl	$1, %esi
	xorl	%eax, %eax
	LOCK
#if MUTEX == 0
	cmpxchgl %esi, (%rdi)
#else
	cmpxchgl %esi, MUTEX(%rdi)
#endif
	jnz	12f

13:	decl	READERS_QUEUED(%rdi)
	jmp	2b

5:	xorl	%edx, %edx
	incl	NR_READERS(%rdi)
	je	8f
9:	LOCK
#if MUTEX == 0
	decl	(%rdi)
#else
	decl	MUTEX(%rdi)
#endif
	jne	6f
7:

	movq	%rdx, %rax
	retq

1:	movl	PSHARED(%rdi), %esi
#if MUTEX != 0
	addq	$MUTEX, %rdi
#endif
	callq	__lll_lock_wait
#if MUTEX != 0
	subq	$MUTEX, %rdi
#endif
	jmp	2b

14:	cmpl	%fs:TID, %eax
	jne	3b
	/* Deadlock detected.  */
	movl	$EDEADLK, %edx
	jmp	9b

6:	movl	PSHARED(%rdi), %esi
#if MUTEX != 0
	addq	$MUTEX, %rdi
#endif
	callq	__lll_unlock_wake
#if MUTEX != 0
	subq	$MUTEX, %rdi
#endif
	jmp	7b

	/* Overflow.  */
8:	decl	NR_READERS(%rdi)
	movl	$EAGAIN, %edx
	jmp	9b

	/* Overflow.  */
4:	decl	READERS_QUEUED(%rdi)
	movl	$EAGAIN, %edx
	jmp	9b

10:	movl	PSHARED(%rdi), %esi
#if MUTEX != 0
	addq	$MUTEX, %rdi
#endif
	callq	__lll_unlock_wake
#if MUTEX != 0
	subq	$MUTEX, %rdi
#endif
	jmp	11b

12:	movl	PSHARED(%rdi), %esi
#if MUTEX == 0
	addq	$MUTEX, %rdi
#endif
	callq	__lll_lock_wait
#if MUTEX != 0
	subq	$MUTEX, %rdi
#endif
	jmp	13b
#endif
	cfi_endproc
	.size	__pthread_rwlock_rdlock,.-__pthread_rwlock_rdlock

	.globl	pthread_rwlock_rdlock
pthread_rwlock_rdlock = __pthread_rwlock_rdlock

	.globl	__pthread_rwlock_rdlock_internal
__pthread_rwlock_rdlock_internal = __pthread_rwlock_rdlock
