/* Copyright (C) 2002 Free Software Foundation, Inc.
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

#define SYS_futex	240
#define FUTEX_WAIT	0
#define FUTEX_WAKE	1

#ifndef UP
# define LOCK lock
#else
# define LOCK
#endif

#define CURR_EVENT	0
#define MUTEX		4
#define LEFT		8
#define	INIT_COUNT	12


	.text

	.globl	pthread_barrier_wait
	.type	pthread_barrier_wait,@function
	.align	16
pthread_barrier_wait:
	pushl	%esi
	pushl	%ebx

	movl	12(%esp), %ebx
	xorl	%esi, %esi

	/* Get the mutex.  */
	orl	$-1, %eax
	LOCK
	xaddl	%eax, MUTEX(%ebx)
	jne	1f

	/* One less waiter.  If this was the last one needed wake
	   everybody.  */
2:	decl	LEFT(%ebx)
	je	3f

	/* There are more threads to come.  */
	movl	CURR_EVENT(%ebx), %edx

	/* Release the mutex.  */
	LOCK
	incl	MUTEX(%ebx)
	jng	6f

	/* Wait for the remaining threads.  The call will return immediately
	   if the CURR_EVENT memory has meanwhile been changed.  */
7:	movl	%esi, %ecx		/* movl $FUTEX_WAIT, %ecx */
8:	movl	$SYS_futex, %eax
	ENTER_KERNEL

	/* Don't return on spurious wakeups.  The syscall does not change
	   any register except %eax so there is no need to reload any of
	   them.  */
	cmpl	%edx, CURR_EVENT(%ebx)
	je,pn	8b

	/* Note: %esi is still zero.  */
	movl	%esi, %eax		/* != PTHREAD_BARRIER_SERIAL_THREAD */

	popl	%ebx
	popl	%esi
	ret

	/* The necessary number of threads arrived.  */
3:	movl	INIT_COUNT(%ebx), %eax
	movl	%eax, LEFT(%ebx)
	incl	CURR_EVENT(%ebx)

	/* Wake up all waiters.  The count is a signed number in the kernel
	   so 0x7fffffff is the highest value.  */
	movl	$0x7fffffff, %edx
	movl	$FUTEX_WAKE, %ecx
	movl	$SYS_futex, %eax
	ENTER_KERNEL

	/* Release the mutex.  */
	LOCK
	incl	MUTEX(%ebx)
	jng	4f

5:	orl	$-1, %eax		/* == PTHREAD_BARRIER_SERIAL_THREAD */

	popl	%ebx
	popl	%esi
	ret

1:	leal	MUTEX(%ebx), %ecx
	call	__lll_lock_wait
	jmp	2b

4:	leal	MUTEX(%ebx), %eax
	call	__lll_unlock_wake
	jmp	5b

6:	leal	MUTEX(%ebx), %eax
	call	__lll_unlock_wake
	jmp	7b
	.size	pthread_barrier_wait,.-pthread_barrier_wait
