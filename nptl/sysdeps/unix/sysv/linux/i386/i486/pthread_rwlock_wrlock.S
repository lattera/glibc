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


#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1

#ifndef UP
# define LOCK lock
#else
# define LOCK
#endif


	.text

	.globl	__pthread_rwlock_wrlock
	.type	__pthread_rwlock_wrlock,@function
	.align	16
__pthread_rwlock_wrlock:
	pushl	%esi
	pushl	%ebx

	xorl	%esi, %esi
	movl	12(%esp), %ebx

	/* Get the lock.  */
	movl	$1, %edx
	xorl	%eax, %eax
	LOCK
#if MUTEX == 0
	cmpxchgl %edx, (%ebx)
#else
	cmpxchgl %edx, MUTEX(%ebx)
#endif
	jnz	1f

2:	movl	WRITER(%ebx), %eax
	testl	%eax, %eax
	jne	14f
	cmpl	$0, NR_READERS(%ebx)
	je	5f

3:	addl	$1, WRITERS_QUEUED(%ebx)
	je	4f

	movl	WRITERS_WAKEUP(%ebx), %edx

	LOCK
#if MUTEX == 0
	subl	$1, (%ebx)
#else
	subl	$1, MUTEX(%ebx)
#endif
	jne	10f

11:	addl	$WRITERS_WAKEUP, %ebx
	movl	%esi, %ecx	/* movl $FUTEX_WAIT, %ecx */
	movl	$SYS_futex, %eax
	ENTER_KERNEL

	subl	$WRITERS_WAKEUP, %ebx

	/* Reget the lock.  */
	movl	$1, %edx
	xorl	%eax, %eax
	LOCK
#if MUTEX == 0
	cmpxchgl %edx, (%ebx)
#else
	cmpxchgl %edx, MUTEX(%ebx)
#endif
	jnz	12f

13:	subl	$1, WRITERS_QUEUED(%ebx)
	jmp	2b

5:	xorl	%ecx, %ecx
	movl	%gs:TID, %eax
	movl	%eax, WRITER(%ebx)
9:	LOCK
#if MUTEX == 0
	subl	$1, (%ebx)
#else
	subl	$1, MUTEX(%ebx)
#endif
	jne	6f
7:

	movl	%ecx, %eax
	popl	%ebx
	popl	%esi
	ret

1:
#if MUTEX == 0
	movl	%ebx, %ecx
#else
	leal	MUTEX(%ebx), %ecx
#endif
	call	__lll_mutex_lock_wait
	jmp	2b

14:	cmpl	%gs:TID	, %eax
	jne	3b
	movl	$EDEADLK, %ecx
	jmp	9b

6:
#if MUTEX == 0
	movl	%ebx, %eax
#else
	leal	MUTEX(%ebx), %eax
#endif
	call	__lll_mutex_unlock_wake
	jmp	7b

4:	subl	$1, WRITERS_QUEUED(%ebx)
	movl	$EAGAIN, %ecx
	jmp	9b

10:
#if MUTEX == 0
	movl	%ebx, %eax
#else
	leal	MUTEX(%ebx), %eax
#endif
	call	__lll_mutex_unlock_wake
	jmp	11b

12:
#if MUTEX == 0
	movl	%ebx, %ecx
#else
	leal	MUTEX(%ebx), %ecx
#endif
	call	__lll_mutex_lock_wait
	jmp	13b
	.size	__pthread_rwlock_wrlock,.-__pthread_rwlock_wrlock

	.globl	pthread_rwlock_wrlock
pthread_rwlock_wrlock = __pthread_rwlock_wrlock

	.globl	__pthread_rwlock_wrlock_internal
__pthread_rwlock_wrlock_internal = __pthread_rwlock_wrlock
