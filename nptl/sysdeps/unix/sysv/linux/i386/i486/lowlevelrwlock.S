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

	.text

#define SYS_gettimeofday	__NR_gettimeofday
#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1

#define EAGAIN		11
#define EDEADLK		35
#define ETIMEDOUT	110

/* Offsets in the pthread_rwlock_t structure.  */
#define MUTEX		0
#define NR_READERS	4
#define READERS_WAKEUP	8
#define WRITERS_WAKEUP	12
#define READERS_QUEUED	16
#define WRITERS_QUEUED	20
#define FLAGS		24
#define WRITER	28

#ifndef UP
# define LOCK lock
#else
# define LOCK
#endif


	.globl	__pthread_rwlock_rdlock
	.type	__pthread_rwlock_rdlock,@function
	.align	16
__pthread_rwlock_rdlock:
	pushl	%esi
	pushl	%ebx

	xorl	%esi, %esi
	xorl	%edx, %edx
	movl	12(%esp), %ebx

	/* Get the lock.  */
	movl	$1, %eax
	LOCK
	xaddl	%eax, MUTEX(%ebx)
	testl	%eax, %eax
	jne	1f

2:	movl	WRITER(%ebx), %eax
	testl	%eax, %eax
	jne	14f
	cmp	$0, WRITERS_QUEUED(%ebx)
	je	5f
	cmpl	$0, FLAGS(%ebx)
	je	5f

3:	incl	READERS_QUEUED(%ebx)
	je	4f

	LOCK
	decl	MUTEX(%ebx)
	jne	10f

11:	addl	$READERS_WAKEUP-MUTEX, %ebx
	movl	%esi, %ecx	/* movl $FUTEX_WAIT, %ecx */
	movl	$SYS_futex, %eax
	int	$0x80

	subl	$READERS_WAKEUP-MUTEX, %ebx

	/* Reget the lock.  */
	movl	$1, %eax
	LOCK
	xaddl	%eax, MUTEX(%ebx)
	testl	%eax, %eax
	jne	12f

13:	decl	READERS_QUEUED(%ebx)
	jne	2b
	movl	$0, READERS_WAKEUP(%ebx)
	jmp	2b

5:	xorl	%ecx, %ecx
	incl	NR_READERS(%ebx)
	je	8f
9:	LOCK
	decl	MUTEX(%ebx)
	jne	6f
7:

	movl	%ecx, %eax
	popl	%ebx
	popl	%esi
	ret

1:	movl	%ebx, %ecx
	call	__lll_mutex_lock_wait
	jmp	2b

14:	cmpl	%gs:8, %eax
	jne	3b
	/* Deadlock detected.  */
	movl	$EDEADLK, %ecx
	jmp	9b

6:	movl	%ebx, %eax
	call	__lll_mutex_unlock_wake
	jmp	7b

	/* Overflow.  */
8:	decl	NR_READERS(%ebx)
	movl	$EAGAIN, %ecx
	jmp	9b

	/* Overflow.  */
4:	decl	READERS_QUEUED(%ebx)
	movl	$EAGAIN, %ecx
	jmp	9b

10:	movl	%ebx, %eax
	call	__lll_mutex_unlock_wake
	jmp	11b

12:	movl	%ebx, %ecx
	call	__lll_mutex_lock_wait
	jmp	13b
	.size	__pthread_rwlock_rdlock,.-__pthread_rwlock_rdlock

	.globl	pthread_rwlock_rdlock
pthread_rwlock_rdlock = __pthread_rwlock_rdlock


	.globl	pthread_rwlock_timedrdlock
	.type	pthread_rwlock_timedrdlock,@function
	.align	16
pthread_rwlock_timedrdlock:
	pushl	%esi
	pushl	%edi
	pushl	%ebx
	pushl	%ebp
	subl	$8, %esp

	movl	28(%esp), %ebp
	movl	32(%esp), %edi

	/* Get the lock.  */
	movl	$1, %eax
	LOCK
	xaddl	%eax, MUTEX(%ebp)
	testl	%eax, %eax
	jne	1f

2:	movl	WRITER(%ebp), %eax
	testl	%eax, %eax
	jne	14f
	cmp	$0, WRITERS_QUEUED(%ebp)
	je	5f
	cmpl	$0, FLAGS(%ebp)
	je	5f

3:	incl	READERS_QUEUED(%ebp)
	je	4f

	LOCK
	decl	MUTEX(%ebp)
	jne	10f

	/* Get current time.  */
	movl	%esp, %ebx
	xorl	%ecx, %ecx
	movl	$SYS_gettimeofday, %eax
	int	$0x80

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
	decl	%ecx
15:	testl	%ecx, %ecx
	js	16f		/* Time is already up.  */

	/* Futex call.  */
	movl	%ecx, (%esp)	/* Store relative timeout.  */
	movl	%edx, 4(%esp)
	xorl	%ecx, %ecx	/* movl $FUTEX_WAIT, %ecx */
	movl	%esp, %esi
	movl	%ecx, %edx
	leal	READERS_WAKEUP(%ebp), %ebx
	movl	$SYS_futex, %eax
	int	$0x80
	movl	%eax, %edx
17:

	/* Reget the lock.  */
	movl	$1, %eax
	LOCK
	xaddl	%eax, MUTEX(%ebp)
	testl	%eax, %eax
	jne	12f

13:	cmpl	$-ETIMEDOUT, %ecx
	je	18f
	decl	READERS_QUEUED(%ebp)
	jne	2b
	movl	$0, READERS_WAKEUP(%ebp)
	jmp	2b


5:	xorl	%ecx, %ecx
	incl	NR_READERS(%ebp)
	je	8f
9:	LOCK
	decl	MUTEX(%ebp)
	jne	6f

7:	movl	%ecx, %eax

	addl	$8, %esp
	popl	%ebp
	popl	%ebx
	popl	%edi
	popl	%esi
	ret

1:	movl	%ebp, %ecx
	call	__lll_mutex_lock_wait
	jmp	2b

14:	cmpl	%gs:8, %eax
	jne	3b
	movl	$EDEADLK, %ecx
	jmp	9b

6:	movl	%ebp, %eax
	call	__lll_mutex_unlock_wake
	jmp	7b

	/* Overflow.  */
8:	decl	NR_READERS(%ebp)
	movl	$EAGAIN, %ecx
	jmp	9b

	/* Overflow.  */
4:	decl	READERS_QUEUED(%ebp)
	movl	$EAGAIN, %ecx
	jmp	9b

10:	movl	%ebp, %eax
	call	__lll_mutex_unlock_wake
	jmp	11b

12:	movl	%ebx, %ecx
	call	__lll_mutex_lock_wait
	jmp	13b

16:	movl	$-ETIMEDOUT, %ecx
	jmp	17b

18:	movl	$ETIMEDOUT, %ecx
	jmp	9b
	.size	pthread_rwlock_timedrdlock,.-pthread_rwlock_timedrdlock


	.globl	__pthread_rwlock_wrlock
	.type	__pthread_rwlock_wrlock,@function
	.align	16
__pthread_rwlock_wrlock:
	pushl	%esi
	pushl	%ebx

	xorl	%esi, %esi
	xorl	%edx, %edx
	movl	12(%esp), %ebx

	/* Get the lock.  */
	movl	$1, %eax
	LOCK
	xaddl	%eax, MUTEX(%ebx)
	testl	%eax, %eax
	jne	1f

2:	movl	WRITER(%ebx), %eax
	testl	%eax, %eax
	jne	14f
	cmp	$0, NR_READERS(%ebx)
	je	5f

3:	incl	WRITERS_QUEUED(%ebx)
	je	4f

	LOCK
	decl	MUTEX(%ebx)
	jne	10f

11:	addl	$WRITERS_WAKEUP-MUTEX, %ebx
	movl	%esi, %ecx	/* movl $FUTEX_WAIT, %ecx */
	movl	$SYS_futex, %eax
	int	$0x80

	subl	$WRITERS_WAKEUP-MUTEX, %ebx

	/* Reget the lock.  */
	movl	$1, %eax
	LOCK
	xaddl	%eax, MUTEX(%ebx)
	testl	%eax, %eax
	jne	12f

13:	decl	WRITERS_QUEUED(%ebx)
	movl	$0, WRITERS_WAKEUP(%ebx)
	jmp	2b

5:	xorl	%ecx, %ecx
	movl	%gs:8, %eax
	movl	%eax, WRITER(%ebx)
9:	LOCK
	decl	MUTEX(%ebx)
	jne	6f
7:

	movl	%ecx, %eax
	popl	%ebx
	popl	%esi
	ret

1:	movl	%ebx, %ecx
	call	__lll_mutex_lock_wait
	jmp	2b

14:	cmpl	%gs:8, %eax
	jne	3b
	movl	$EDEADLK, %ecx
	jmp	9b

6:	movl	%ebx, %eax
	call	__lll_mutex_unlock_wake
	jmp	7b

4:	decl	WRITERS_QUEUED(%ebx)
	movl	$EAGAIN, %ecx
	jmp	9b

10:	movl	%ebx, %eax
	call	__lll_mutex_unlock_wake
	jmp	11b

12:	movl	%ebx, %ecx
	call	__lll_mutex_lock_wait
	jmp	13b
	.size	__pthread_rwlock_wrlock,.-__pthread_rwlock_wrlock

	.globl	pthread_rwlock_wrlock
pthread_rwlock_wrlock = __pthread_rwlock_wrlock


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
	movl	$1, %eax
	LOCK
	xaddl	%eax, MUTEX(%ebp)
	testl	%eax, %eax
	jne	1f

2:	movl	WRITER(%ebp), %eax
	testl	%eax, %eax
	jne	14f
	cmp	$0, NR_READERS(%ebp)
	je	5f

3:	incl	WRITERS_QUEUED(%ebp)
	je	4f

	LOCK
	decl	MUTEX(%ebp)
	jne	10f

	/* Get current time.  */
	movl	%esp, %ebx
	xorl	%ecx, %ecx
	movl	$SYS_gettimeofday, %eax
	int	$0x80

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
	decl	%ecx
15:	testl	%ecx, %ecx
	js	16f		/* Time is already up.  */

	/* Futex call.  */
	movl	%ecx, (%esp)	/* Store relative timeout.  */
	movl	%edx, 4(%esp)
	xorl	%ecx, %ecx	/* movl $FUTEX_WAIT, %ecx */
	movl	%esp, %esi
	movl	%ecx, %edx
	leal	WRITERS_WAKEUP(%ebp), %ebx
	movl	$SYS_futex, %eax
	int	$0x80
	movl	%eax, %edx
17:

	/* Reget the lock.  */
	movl	$1, %eax
	LOCK
	xaddl	%eax, MUTEX(%ebp)
	testl	%eax, %eax
	jne	12f

13:	cmpl	$-ETIMEDOUT, %ecx
	je	18f
	decl	WRITERS_QUEUED(%ebp)
	movl	$0, WRITERS_WAKEUP(%ebp)
	jmp	2b


5:	xorl	%ecx, %ecx
	movl	%gs:8, %eax
	movl	%eax, WRITER(%ebp)
9:	LOCK
	decl	MUTEX(%ebp)
	jne	6f

7:	movl	%ecx, %eax

	addl	$8, %esp
	popl	%ebp
	popl	%ebx
	popl	%edi
	popl	%esi
	ret

1:	movl	%ebp, %ecx
	call	__lll_mutex_lock_wait
	jmp	2b

14:	cmpl	%gs:8, %eax
	jne	3b
	movl	$EDEADLK, %ecx
	jmp	9b

6:	movl	%ebp, %eax
	call	__lll_mutex_unlock_wake
	jmp	7b

	/* Overflow.  */
4:	decl	WRITERS_QUEUED(%ebp)
	movl	$EAGAIN, %ecx
	jmp	9b

10:	movl	%ebp, %eax
	call	__lll_mutex_unlock_wake
	jmp	11b

12:	movl	%ebx, %ecx
	call	__lll_mutex_lock_wait
	jmp	13b

16:	movl	$-ETIMEDOUT, %ecx
	jmp	17b

18:	movl	$ETIMEDOUT, %ecx
	jmp	9b
	.size	pthread_rwlock_timedwrlock,.-pthread_rwlock_timedwrlock

	.globl	__pthread_rwlock_unlock
	.type	__pthread_rwlock_unlock,@function
	.align	16
__pthread_rwlock_unlock:
	pushl	%ebx
	pushl	%esi
	pushl	%edi

	xorl	%esi, %esi
	xorl	%edx, %edx
	movl	16(%esp), %edi

	/* Get the lock.  */
	movl	$1, %eax
	LOCK
	xaddl	%eax, MUTEX(%edi)
	testl	%eax, %eax
	jne	1f

2:	cmpl	$0, WRITER(%edi)
	jne	5f
	decl	NR_READERS(%edi)
	jnz	6f

5:	movl	$0, WRITER(%edi)

	movl	$0x7fffffff, %edx
	leal	READERS_WAKEUP(%edi), %ebx
	movl	$1, %ecx
	leal	WRITERS_WAKEUP(%edi), %eax
	cmpl	$0, WRITERS_QUEUED(%edi)
	cmovne	%ecx, %edx
	cmovne	%eax, %ebx
	movl	$SYS_futex, %eax
	int	$0x80

6:	LOCK
	decl	MUTEX(%edi)
	jne	3f

4:	xorl	%eax, %eax
	popl	%edi
	popl	%esi
	popl	%ebx
	ret

1:	movl	%edi, %ecx
	call	__lll_mutex_lock_wait
	jmp	2b

3:	movl	%edi, %eax
	call	__lll_mutex_unlock_wake
	jmp	4b

	.size	__pthread_rwlock_unlock,.-__pthread_rwlock_unlock

	.globl	pthread_rwlock_unlock
pthread_rwlock_unlock = __pthread_rwlock_unlock
