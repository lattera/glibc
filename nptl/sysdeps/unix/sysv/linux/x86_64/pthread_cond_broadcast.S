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

#ifdef UP
# define LOCK
#else
# define LOCK lock
#endif

#define SYS_futex		202
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1


	.text

	/* int pthread_cond_broadcast (pthread_cond_t *cond) */
	.globl	__pthread_cond_broadcast
	.type	__pthread_cond_broadcast, @function
	.align	16
__pthread_cond_broadcast:

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

2:	addq	$wakeup_seq, %rdi
	movq	total_seq-wakeup_seq(%rdi), %rcx
	cmpq	(%rdi), %rcx
	jna	4f

	/* Cause all currently waiting threads to recognize they are
	   woken up.  */
	movq	%rcx, (%rdi)

	/* Unlock.  */
	LOCK
	decl	cond_lock-wakeup_seq(%rdi)
	jne	7f

	/* Wake up all threads.  */
8:	movq	$FUTEX_WAKE, %rsi
	movq	$SYS_futex, %rax
	movl	$0x7fffffff, %edx
	syscall

	xorl	%eax, %eax
	retq

	.align	16
	/* Unlock.  */
4:	LOCK
	decl	cond_lock-wakeup_seq(%rdi)
	jne	5f

6:	xorl	%eax, %eax
	retq

	/* Initial locking failed.  */
1:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	call	__lll_mutex_lock_wait
#if cond_lock != 0
	subq	$cond_lock, %rdi
#endif
	jmp	2b

	/* Unlock in loop requires waekup.  */
5:	addq	$cond_lock-wakeup_seq, %rdi
	call	__lll_mutex_unlock_wake
	jmp	6b

	/* Unlock in loop requires waekup.  */
7:	addq	$cond_lock-wakeup_seq, %rdi
	call	__lll_mutex_unlock_wake
	subq	$cond_lock-wakeup_seq, %rdi
	jmp	8b
	.size	__pthread_cond_broadcast, .-__pthread_cond_broadcast
versioned_symbol (libpthread, __pthread_cond_broadcast, pthread_cond_broadcast,
		  GLIBC_2_3_2)
