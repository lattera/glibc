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
#include <lowlevelbarrier.h>

#define SYS_futex	202
#define FUTEX_WAIT	0
#define FUTEX_WAKE	1

#ifndef UP
# define LOCK lock
#else
# define LOCK
#endif


	.text

	.globl	pthread_barrier_wait
	.type	pthread_barrier_wait,@function
	.align	16
pthread_barrier_wait:
	/* Get the mutex.  */
	orl	$-1, %esi
	LOCK
	xaddl	%esi, MUTEX(%rdi)
	jne	1f

	/* One less waiter.  If this was the last one needed wake
	   everybody.  */
2:	decl	LEFT(%rdi)
	je	3f

	/* There are more threads to come.  */
#if CURR_EVENT == 0
	movl	(%rdi), %edx
#else
	movl	CURR_EVENT(%rdi), %edx
#endif

	/* Release the mutex.  */
	LOCK
	addl	$1, MUTEX(%rdi)
	jng	6f

	/* Wait for the remaining threads.  The call will return immediately
	   if the CURR_EVENT memory has meanwhile been changed.  */
7:	xorq	%rsi, %rsi		/* movq $FUTEX_WAIT, %rsi */
	xorq	%r10, %r10
8:	movq	$SYS_futex, %rax
	syscall

	/* Don't return on spurious wakeups.  The syscall does not change
	   any register except %eax so there is no need to reload any of
	   them.  */
#if CURR_EVENT == 0
	cmpl	%edx, (%rdi)
#else
	cmpl	%edx, CURR_EVENT(%rdi)
#endif
	je	8b

	/* Note: %esi is still zero.  */
	movl	%esi, %eax		/* != PTHREAD_BARRIER_SERIAL_THREAD */

	retq

	/* The necessary number of threads arrived.  */
3:	movl	INIT_COUNT(%rdi), %eax
	movl	%eax, LEFT(%rdi)
#if CURR_EVENT == 0
	incl	(%rdi)
#else
	incl	CURR_EVENT(%rdi)
#endif

	/* Wake up all waiters.  The count is a signed number in the kernel
	   so 0x7fffffff is the highest value.  */
	movl	$0x7fffffff, %edx
	movq	$FUTEX_WAKE, %rsi
	movq	$SYS_futex, %rax
	syscall

	/* Release the mutex.  We cannot release the lock before
	   waking the waiting threads since otherwise a new thread might
	   arrive and gets waken up, too.  */
	LOCK
	addl	$1, MUTEX(%rdi)
	jng	4f

5:	orl	$-1, %eax		/* == PTHREAD_BARRIER_SERIAL_THREAD */

	retq

1:	addq	$MUTEX, %rdi
	callq	__lll_lock_wait
	subq	$MUTEX, %rdi
	jmp	2b

4:	addq	$MUTEX, %rdi
	callq	__lll_unlock_wake
	subq	$MUTEX, %rdi
	jmp	5b

6:	addq	$MUTEX, %rdi
	callq	__lll_unlock_wake
	subq	$MUTEX, %rdi
	jmp	7b
	.size	pthread_barrier_wait,.-pthread_barrier_wait
