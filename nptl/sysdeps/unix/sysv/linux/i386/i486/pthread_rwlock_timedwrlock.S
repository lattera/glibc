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
#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1

#ifndef UP
# define LOCK lock
#else
# define LOCK
#endif


	.text

	.globl	pthread_rwlock_timedwrlock
	.type	pthread_rwlock_timedwrlock,@function
	.align	16
pthread_rwlock_timedwrlock:
	pushl	%esi
	pushl	%edi
	pushl	%ebx
	pushl	%ebp
	subl	$8, %esp

	movl	28(%esp), %ebp
	movl	32(%esp), %edi

	/* Get the lock.  */
	movl	$1, %edx
	xorl	%eax, %eax
	LOCK
#if MUTEX == 0
	cmpxchgl %edx, (%ebp)
#else
	cmpxchgl %edx, MUTEX(%ebp)
#endif
	jnz	1f

2:	movl	WRITER(%ebp), %eax
	testl	%eax, %eax
	jne	14f
	cmpl	$0, NR_READERS(%ebp)
	je	5f

	/* Check the value of the timeout parameter.  */
3:	cmpl	$1000000000, 4(%edi)
	jae	19f

	addl	$1, WRITERS_QUEUED(%ebp)
	je	4f

	movl	WRITERS_WAKEUP(%ebp), %esi

	LOCK
#if MUTEX == 0
	subl	$1, (%ebp)
#else
	subl	$1, MUTEX(%ebp)
#endif
	jne	10f

	/* Get current time.  */
11:	movl	%esp, %ebx
	xorl	%ecx, %ecx
	movl	$SYS_gettimeofday, %eax
	ENTER_KERNEL

	/* Compute relative timeout.  */
	movl	4(%esp), %eax
	movl	$1000, %edx
	mul	%edx		/* Milli seconds to nano seconds.  */
	movl	(%edi), %ecx
	movl	4(%edi), %edx
	subl	(%esp), %ecx
	subl	%eax, %edx
	jns	15f
	addl	$1000000000, %edx
	subl	$1, %ecx
15:	testl	%ecx, %ecx
	js	16f		/* Time is already up.  */

	/* Futex call.  */
	movl	%ecx, (%esp)	/* Store relative timeout.  */
	movl	%edx, 4(%esp)
	movl	%esi, %edx
	xorl	%ecx, %ecx	/* movl $FUTEX_WAIT, %ecx */
	movl	%esp, %esi
	leal	WRITERS_WAKEUP(%ebp), %ebx
	movl	$SYS_futex, %eax
	ENTER_KERNEL
	movl	%eax, %ecx
17:

	/* Reget the lock.  */
	movl	$1, %edx
	xorl	%eax, %eax
	LOCK
#if MUTEX == 0
	cmpxchgl %edx, (%ebp)
#else
	cmpxchgl %edx, MUTEX(%ebp)
#endif
	jnz	12f

13:	subl	$1, WRITERS_QUEUED(%ebp)
	cmpl	$-ETIMEDOUT, %ecx
	jne	2b

18:	movl	$ETIMEDOUT, %ecx
	jmp	9f


5:	xorl	%ecx, %ecx
	movl	%gs:TID, %eax
	movl	%eax, WRITER(%ebp)
9:	LOCK
#if MUTEX == 0
	subl	$1, (%ebp)
#else
	subl	$1, MUTEX(%ebp)
#endif
	jne	6f

7:	movl	%ecx, %eax

	addl	$8, %esp
	popl	%ebp
	popl	%ebx
	popl	%edi
	popl	%esi
	ret

1:
#if MUTEX == 0
	movl	%ebp, %ecx
#else
	leal	MUTEX(%ebp), %ecx
#endif
	call	__lll_mutex_lock_wait
	jmp	2b

14:	cmpl	%gs:TID, %eax
	jne	3b
20:	movl	$EDEADLK, %ecx
	jmp	9b

6:
#if MUTEX == 0
	movl	%ebp, %eax
#else
	leal	MUTEX(%ebp), %eax
#endif
	call	__lll_mutex_unlock_wake
	jmp	7b

	/* Overflow.  */
4:	subl	$1, WRITERS_QUEUED(%ebp)
	movl	$EAGAIN, %ecx
	jmp	9b

10:
#if MUTEX == 0
	movl	%ebp, %eax
#else
	leal	MUTEX(%ebp), %eax
#endif
	call	__lll_mutex_unlock_wake
	jmp	11b

12:
#if MUTEX == 0
	movl	%ebp, %ecx
#else
	leal	MUTEX(%ebp), %ecx
#endif
	call	__lll_mutex_lock_wait
	jmp	13b

16:	movl	$-ETIMEDOUT, %ecx
	jmp	17b

19:	movl	$EINVAL, %ecx
	jmp	9b
	.size	pthread_rwlock_timedwrlock,.-pthread_rwlock_timedwrlock
