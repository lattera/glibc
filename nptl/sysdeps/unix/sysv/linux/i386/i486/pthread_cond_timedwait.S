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

#define SYS_gettimeofday	__NR_gettimeofday
#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1

#define ETIMEDOUT		110


	.text

/* int pthread_cond_timedwait (pthread_cond_t *cond, pthread_mutex_t *mutex,
			       const struct timespec *abstime)  */
	.globl	__pthread_cond_timedwait
	.type	__pthread_cond_timedwait, @function
	.align	16
__pthread_cond_timedwait:

	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx

	movl	20(%esp), %ebx
	movl	28(%esp), %ebp

	/* Get internal lock.  */
	movl	$1, %eax
	LOCK
#if cond_lock == 0
	xaddl	%eax, (%ebx)
#else
	xaddl	%eax, cond_lock(%ebx)
#endif
	testl	%eax, %eax
	jne	1f

	/* Unlock the mutex.  */
2:	pushl	24(%esp)
	call	__pthread_mutex_unlock_internal

	testl	%eax, %eax
	jne	16f

	addl	$1, total_seq(%ebx)
	adcl	$0, total_seq+4(%ebx)

	/* Install cancellation handler.  */
#ifdef PIC
	call	__i686.get_pc_thunk.cx
	addl	$_GLOBAL_OFFSET_TABLE_, %ecx
	leal	__condvar_cleanup@GOTOFF(%ecx), %eax
#else
	leal	__condvar_cleanup, %eax
#endif
	subl	$40, %esp
	leal	28(%esp), %edx
	movl	%esp, 8(%esp)
	movl	%eax, 4(%esp)
	movl	%edx, (%esp)
	call	__pthread_cleanup_push

	/* Address of the mutex.  */
	movl	68(%esp), %ecx
	/* Get and store current wakeup_seq value.  */
	movl	wakeup_seq(%ebx), %edi
	movl	wakeup_seq+4(%ebx), %edx
	movl	%edi, 20(%esp)
	movl	%edx, 24(%esp)
	/* Prepare structure passed to cancellation handler.  */
	movl	%ebx, 4(%esp)
	movl	%ecx, 8(%esp)

	/* Unlock.  */
8:	LOCK
#if cond_lock == 0
	subl	$1, (%ebx)
#else
	subl	$1, cond_lock(%ebx)
#endif
	jne	3f

4:	movl	%esp, %eax
	call	__pthread_enable_asynccancel_2

	/* Get the current time.  */
	movl	%ebx, %edx
	leal	12(%esp), %ebx
	xorl	%ecx, %ecx
	movl	$SYS_gettimeofday, %eax
	ENTER_KERNEL
	movl	%edx, %ebx

	/* Compute relative timeout.  */
	movl	16(%esp), %eax
	movl	$1000, %edx
	mul	%edx		/* Milli seconds to nano seconds.  */
	movl	(%ebp), %ecx
	movl	4(%ebp), %edx
	subl	12(%esp), %ecx
	subl	%eax, %edx
	jns	12f
	addl	$1000000000, %edx
	subl	$1, %ecx
12:	testl	%ecx, %ecx
	js	13f

	/* Store relative timeout.  */
	movl	%ecx, 12(%esp)
	movl	%edx, 16(%esp)
	leal	12(%esp), %esi
	xorl	%ecx, %ecx	/* movl $FUTEX_WAIT, %ecx */
	movl	%edi, %edx
	addl	$wakeup_seq, %ebx
	movl	$SYS_futex, %eax
	ENTER_KERNEL
	subl	$wakeup_seq, %ebx
	movl	%eax, %esi

	movl	(%esp), %eax
	call	__pthread_disable_asynccancel

	/* Lock.  */
	movl	$1, %eax
	LOCK
#if cond_lock == 0
	xaddl	%eax, (%ebx)
#else
	xaddl	%eax, cond_lock(%ebx)
#endif
	testl	%eax, %eax
	jne	5f

6:	movl	woken_seq(%ebx), %eax
	movl	woken_seq+4(%ebx), %ecx

	movl	wakeup_seq(%ebx), %edi
	movl	wakeup_seq+4(%ebx), %edx

	cmpl	24(%esp), %ecx
	ja	7f
	jb	15f
	cmpl	20(%esp), %eax
	jb	15f

7:	cmpl	%ecx, %edx
	ja	9f
	jb	15f
	cmp	%eax, %edi
	ja	9f

15:	cmpl	$-ETIMEDOUT, %esi
	jne	8b

13:	addl	$1, wakeup_seq(%ebx)
	adcl	$0, wakeup_seq+4(%ebx)
	movl	$ETIMEDOUT, %esi
	jmp	14f

9:	xorl	%esi, %esi
14:	addl	$1, woken_seq(%ebx)
	adcl	$0, woken_seq+4(%ebx)

	LOCK
#if cond_lock == 0
	subl	$1, (%ebx)
#else
	subl	$1, cond_lock(%ebx)
#endif
	jne	10f

	/* Remove cancellation handler.  */
11:	movl	20+CLEANUP_PREV(%esp), %edx
	movl	%edx, %gs:CLEANUP

	/* Trick ahead:	 8(%esp) contains the address of the mutex.  */
	addl	$8, %esp
	call	__pthread_mutex_lock_internal
	addl	$36, %esp

	movl	%esi, %eax

18:	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp

	/* We return the result of the mutex_lock operation.  */
	ret

	/* Initial locking failed.  */
1:
#if cond_lock == 0
	movl	%ebx, %ecx
#else
	leal	cond_lock(%ebx), %ecx
#endif
	call	__lll_mutex_lock_wait
	jmp	2b

	/* Unlock in loop requires waekup.  */
3:
#if cond_lock == 0
	movl	%ebx, %eax
#else
	leal	cond_lock(%ebx), %eax
#endif
	call	__lll_mutex_unlock_wake
	jmp	4b

	/* Locking in loop failed.  */
5:
#if cond_lock == 0
	movl	%ebx, %ecx
#else
	leal	cond_lock(%ebx), %ecx
#endif
	call	__lll_mutex_lock_wait
	jmp	6b

	/* Unlock after loop requires waekup.  */
10:
#if cond_lock == 0
	movl	%ebx, %eax
#else
	leal	cond_lock(%ebx), %eax
#endif
	call	__lll_mutex_unlock_wake
	jmp	11b

	/* The initial unlocking of the mutex failed.  */
16:	movl	%eax, (%esp)
	LOCK
#if cond_lock == 0
	subl	$1, (%ebx)
#else
	subl	$1, cond_lock(%ebx)
#endif
	jne	17f

#if cond_lock == 0
	movl	%ebx, %eax
#else
	leal	cond_lock(%ebx), %eax
#endif
	call	__lll_mutex_unlock_wake

17:	popl	%eax
	jmp	18b
	.size	__pthread_cond_wait, .-__pthread_cond_wait
	.size	__pthread_cond_timedwait, .-__pthread_cond_timedwait
versioned_symbol (libpthread, __pthread_cond_timedwait, pthread_cond_timedwait,
		  GLIBC_2_3_2)
