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

#define cond_lock	0
#define total_seq	4
#define wakeup_seq	12
#define woken_seq	20


	.text

	.align	16
	.type	condvar_cleanup, @function
condvar_cleanup:
	pushl	%ebx
	movl	4(%esp), %ebx
#if cond_lock != 0
	addl	$cond_lock, %ebx
#endif

	/* Get internal lock.  */
	movl	$1, %eax
	LOCK
#if cond_lock == 0
	xaddl	%eax, (%ebx)
#else
	xaddl	%eax, cond_lock(%ebx)
#endif
	testl	%eax, %eax
	je	1f

#if cond_lock == 0
	movl	%ebx, %ecx
#else
	leal	cond_lock(%ebx), %ecx
#endif
	call	__lll_mutex_lock_wait

1:	addl	$1, wakeup_seq(%ebx)
	adcl	$0, wakeup_seq+4(%ebx)

	addl	$1, woken_seq(%ebx)
	adcl	$0, woken_seq+4(%ebx)

	LOCK
	decl	(%ebx)
	je	2f
#if cond_lock == 0
	movl	%ebx, %eax
#else
	leal	cond_lock(%ebx), %eax
#endif
	call	__lll_mutex_unlock_wake

2:	popl	%ebx
	ret
	.size	condvar_cleanup, .-condvar_cleanup


/* int pthread_cond_wait (pthread_cond_t *cond, pthread_mutex_t *mutex)  */
	.globl	__pthread_cond_wait
	.type	__pthread_cond_wait, @function
	.align	16
__pthread_cond_wait:

	pushl	%edi
	pushl	%esi
	pushl	%ebx

	xorl	%esi, %esi
	movl	16(%esp), %ebx
#if cond_lock != 0
	addl	$cond_lock, %ebx
#endif

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
2:	pushl	20(%esp)
	call	__pthread_mutex_unlock_internal

	addl	$1, total_seq(%ebx)
	adcl	$0, total_seq+4(%ebx)

	/* Install cancellation handler.  */
#ifdef PIC
	call	__i686.get_pc_thunk.cx
	addl	$_GLOBAL_OFFSET_TABLE_, %ecx
	leal	condvar_cleanup@GOTOFF(%ecx), %eax
#else
	leal	condvar_cleanup, %eax
#endif
	subl	$24, %esp
	leal	12(%esp), %edx
	movl	%ebx, 8(%esp)
	movl	%eax, 4(%esp)
	movl	%edx, (%esp)
	call	_GI_pthread_cleanup_push

	/* Get and store current wakeup_seq value.  */
	movl	wakeup_seq(%ebx), %edi
	movl	wakeup_seq+4(%ebx), %edx
	movl	%edi, 4(%esp)
	movl	%edx, 8(%esp)

	/* Unlock.  */
8:	LOCK
#if cond_lock == 0
	decl	(%ebx)
#else
	decl	cond_lock(%ebx)
#endif
	jne	3f

4:	call	__pthread_enable_asynccancel
	movl	%eax, (%esp)

	movl	%esi, %ecx	/* movl $FUTEX_WAIT, %ecx */
	movl	%edi, %edx
	addl	$wakeup_seq-cond_lock, %ebx
	movl	$SYS_futex, %eax
	ENTER_KERNEL
	subl	$wakeup_seq-cond_lock, %ebx

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

	cmpl	8(%esp), %ecx
	ja	7f
	jb	8b
	cmpl	4(%esp), %eax
	jb	8b

7:	cmpl	%ecx, %edx
	ja	9f
	jb	8b
	cmp	%eax, %edi
	jna	8b

9:	addl	$1, woken_seq(%ebx)
	adcl	$0, woken_seq+4(%ebx)

	LOCK
#if cond_lock == 0
	decl	(%ebx)
#else
	decl	cond_lock(%ebx)
#endif
	jne	10f

	/* Remove cancellation handler.  */
11:	leal	12(%esp), %edx
	movl	$0, 4(%esp)
	movl	%edx, (%esp)
	call	_GI_pthread_cleanup_pop

	movl	48(%esp), %eax
	movl	%eax, (%esp)
	call	__pthread_mutex_lock_internal
	addl	$28, %esp

	popl	%ebx
	popl	%esi
	popl	%edi

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
	.size	__pthread_cond_wait, .-__pthread_cond_wait
versioned_symbol (libpthread, __pthread_cond_wait, pthread_cond_wait,
		  GLIBC_2_3_2)


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
#if cond_lock != 0
	addl	$cond_lock, %ebx
#endif

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

	addl	$1, total_seq(%ebx)
	adcl	$0, total_seq+4(%ebx)

	/* Install cancellation handler.  */
#ifdef PIC
	call	__i686.get_pc_thunk.cx
	addl	$_GLOBAL_OFFSET_TABLE_, %ecx
	leal	condvar_cleanup@GOTOFF(%ecx), %eax
#else
	leal	condvar_cleanup, %eax
#endif
	subl	$32, %esp
	leal	16(%esp), %edx
	movl	%ebx, 8(%esp)
	movl	%eax, 4(%esp)
	movl	%edx, (%esp)
	call	_GI_pthread_cleanup_push

	/* Get and store current wakeup_seq value.  */
	movl	wakeup_seq(%ebx), %edi
	movl	wakeup_seq+4(%ebx), %edx
	movl	%edi, 12(%esp)
	movl	%edx, 16(%esp)

	/* Unlock.  */
8:	LOCK
#if cond_lock == 0
	decl	(%ebx)
#else
	decl	cond_lock(%ebx)
#endif
	jne	3f

4:	call	__pthread_enable_asynccancel
	movl	%eax, (%esp)

	/* Get the current time.  */
	movl	%ebx, %edx
	leal	4(%esp), %ebx
	xorl	%ecx, %ecx
	movl	$SYS_gettimeofday, %eax
	ENTER_KERNEL
	movl	%edx, %ebx

	/* Compute relative timeout.  */
	movl	8(%esp), %eax
	movl	$1000, %edx
	mul	%edx		/* Milli seconds to nano seconds.  */
	movl	(%ebp), %ecx
	movl	4(%ebp), %edx
	subl	4(%esp), %ecx
	subl	%eax, %edx
	jns	12f
	addl	$1000000000, %edx
	decl	%ecx
12:	testl	%ecx, %ecx
	js	13f

	/* Store relative timeout.  */
	movl	%ecx, 4(%esp)
	movl	%edx, 8(%esp)
	leal	4(%esp), %esi
	xorl	%ecx, %ecx	/* movl $FUTEX_WAIT, %ecx */
	movl	%edi, %edx
	addl	$wakeup_seq-cond_lock, %ebx
	movl	$SYS_futex, %eax
	ENTER_KERNEL
	subl	$wakeup_seq-cond_lock, %ebx
	movl	%eax, %esi

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

	cmpl	16(%esp), %ecx
	ja	7f
	jb	15f
	cmpl	12(%esp), %eax
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
	decl	(%ebx)
#else
	decl	cond_lock(%ebx)
#endif
	jne	10f

	/* Remove cancellation handler.  */
11:	leal	20(%esp), %edx
	movl	$0, 4(%esp)
	movl	%edx, (%esp)
	call	_GI_pthread_cleanup_pop

	movl	60(%esp), %ecx
	movl	%ecx, (%esp)
	call	__pthread_mutex_lock_internal
	addl	$36, %esp

	movl	%esi, %eax

	popl	%ebx
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
	.size	__pthread_cond_timedwait, .-__pthread_cond_timedwait
versioned_symbol (libpthread, __pthread_cond_timedwait, pthread_cond_timedwait,
		  GLIBC_2_3_2)


	/* int pthread_cond_signal (pthread_cond_t *cond) */
	.globl	__pthread_cond_signal
	.type	__pthread_cond_signal, @function
	.align	16
__pthread_cond_signal:

	pushl	%esi
	pushl	%ebx
#if cond_lock != 0
	addl	$cond_lock, %ebx
#endif

	movl	12(%esp), %ebx

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

2:	movl	total_seq+4(%ebx), %eax
	movl	total_seq(%ebx), %ecx
	cmpl	wakeup_seq+4(%ebx), %eax
	ja	3f
	jb	4f
	cmpl	wakeup_seq(%ebx), %ecx
	jbe	4f

	/* Bump the wakeup number.  */
3:	addl	$1, wakeup_seq(%ebx)
	adcl	$0, wakeup_seq+4(%ebx)

	/* Wake up one thread.  */
	addl	$wakeup_seq-cond_lock, %ebx
	movl	$FUTEX_WAKE, %ecx
	xorl	%esi, %esi
	movl	$SYS_futex, %eax
	movl	%ecx, %edx	/* movl $1, %edx */
	ENTER_KERNEL

	subl	$wakeup_seq-cond_lock, %ebx

	/* Unlock.  */
4:	LOCK
#if cond_lock == 0
	decl	(%ebx)
#else
	decl	cond_lock(%ebx)
#endif
	jne	5f

6:	xorl	%eax, %eax
	popl	%ebx
	popl	%esi
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
5:
#if cond_lock == 0
	movl	%ebx, %eax
#else
	leal	cond_lock(%ebx), %eax
#endif
	call	__lll_mutex_unlock_wake
	jmp	6b
	.size	__pthread_cond_signal, .-__pthread_cond_signal
versioned_symbol (libpthread, __pthread_cond_signal, pthread_cond_signal,
		  GLIBC_2_3_2)


	/* int pthread_cond_broadcast (pthread_cond_t *cond) */
	.globl	__pthread_cond_broadcast
	.type	__pthread_cond_broadcast, @function
	.align	16
__pthread_cond_broadcast:

	pushl	%esi
	pushl	%ebx

	movl	12(%esp), %ebx
#if cond_lock != 0
	addl	$cond_lock, %ebx
#endif

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

2:	movl	total_seq+4(%ebx), %eax
	movl	total_seq(%ebx), %ecx
	cmpl	wakeup_seq+4(%ebx), %eax
	ja	3f
	jb	4f
	cmpl	wakeup_seq(%ebx), %ecx
	jna	4f

	/* Case all currently waiting threads to wake up.  */
3:	movl	%ecx, wakeup_seq(%ebx)
	movl	%eax, wakeup_seq+4(%ebx)

	/* Wake up all threads.  */
	addl	$wakeup_seq-cond_lock, %ebx
	movl	$FUTEX_WAKE, %ecx
	xorl	%esi, %esi
	movl	$SYS_futex, %eax
	movl	$0x7fffffff, %edx
	ENTER_KERNEL

	subl	$wakeup_seq-cond_lock, %ebx

	/* Unlock.  */
4:	LOCK
#if cond_lock == 0
	decl	(%ebx)
#else
	decl	cond_lock(%ebx)
#endif
	jne	5f

6:	xorl	%eax, %eax
	popl	%ebx
	popl	%esi
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
5:
#if cond_lock == 0
	movl	%ebx, %eax
#else
	leal	cond_lock(%ebx), %eax
#endif
	call	__lll_mutex_unlock_wake
	jmp	6b
	.size	__pthread_cond_broadcast, .-__pthread_cond_broadcast
versioned_symbol (libpthread, __pthread_cond_broadcast, pthread_cond_broadcast,
		  GLIBC_2_3_2)


#ifdef PIC
	.section .gnu.linkonce.t.__i686.get_pc_thunk.cx,"ax",@progbits
	.globl	__i686.get_pc_thunk.cx
	.hidden	__i686.get_pc_thunk.cx
	.type	__i686.get_pc_thunk.cx,@function
__i686.get_pc_thunk.cx:
	movl (%esp), %ecx;
	ret
	.size	__i686.get_pc_thunk.cx,.-__i686.get_pc_thunk.cx
#endif
