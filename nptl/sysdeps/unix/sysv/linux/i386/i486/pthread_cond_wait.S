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
#include <tcb-offsets.h>

#ifdef UP
# define LOCK
#else
# define LOCK lock
#endif

#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1


	.text

	.align	16
	.type	__condvar_cleanup, @function
	.globl	__condvar_cleanup
	.hidden	__condvar_cleanup
__condvar_cleanup:
	pushl	%ebx
	pushl	%esi
	movl	12(%esp), %esi

	/* Get internal lock.  */
	movl	4(%esi), %ebx
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

1:	addl	$wakeup_seq, %ebx
	addl	$1, (%ebx)
	adcl	$0, 4(%ebx)

	addl	$1, woken_seq-wakeup_seq(%ebx)
	adcl	$0, woken_seq-wakeup_seq+4(%ebx)

	/* Wake up all waiters to make sure no signal gets lost.  */
	movl	$FUTEX_WAKE, %ecx
	movl	$SYS_futex, %eax
	movl	$0x7fffffff, %edx
	ENTER_KERNEL

	LOCK
	subl	$1, cond_lock-wakeup_seq(%ebx)
	je	2f

	leal	cond_lock-wakeup_seq(%ebx), %eax
	call	__lll_mutex_unlock_wake

	/* Lock the mutex unless asynchronous cancellation is in effect.  */
2:	testl	$2, (%esi)
	jne	3f

	pushl	8(%esi)
	call	__pthread_mutex_lock_internal
	popl	%eax

3:	popl	%esi
	popl	%ebx
	ret
	.size	__condvar_cleanup, .-__condvar_cleanup


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

	testl	%eax, %eax
	jne	12f

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
	subl	$32, %esp
	leal	20(%esp), %edx
	movl	%esp, 8(%esp)
	movl	%eax, 4(%esp)
	movl	%edx, (%esp)
	call	__pthread_cleanup_push

	/* Get and store current wakeup_seq value.  */
	movl	56(%esp), %ecx
	movl	wakeup_seq(%ebx), %edi
	movl	wakeup_seq+4(%ebx), %edx
	movl	%edi, 12(%esp)
	movl	%edx, 16(%esp)
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

4:	call	__pthread_enable_asynccancel
	movl	%eax, (%esp)

	movl	%esi, %ecx	/* movl $FUTEX_WAIT, %ecx */
	movl	%edi, %edx
	addl	$wakeup_seq, %ebx
	movl	$SYS_futex, %eax
	ENTER_KERNEL
	subl	$wakeup_seq, %ebx

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
	jb	8b
	cmpl	12(%esp), %eax
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
	addl	$28, %esp

14:	popl	%ebx
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

	/* Unlock after loop requires wakeup.  */
10:
#if cond_lock == 0
	movl	%ebx, %eax
#else
	leal	cond_lock(%ebx), %eax
#endif
	call	__lll_mutex_unlock_wake
	jmp	11b

	/* The initial unlocking of the mutex failed.  */
12:	movl	%eax, (%esp)
	LOCK
#if cond_lock == 0
	subl	$1, (%ebx)
#else
	subl	$1, cond_lock(%ebx)
#endif
	jne	13f

#if cond_lock == 0
	movl	%ebx, %eax
#else
	leal	cond_lock(%ebx), %eax
#endif
	call	__lll_mutex_unlock_wake

13:	popl	%eax
	jmp	14b
	.size	__pthread_cond_wait, .-__pthread_cond_wait
versioned_symbol (libpthread, __pthread_cond_wait, pthread_cond_wait,
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
