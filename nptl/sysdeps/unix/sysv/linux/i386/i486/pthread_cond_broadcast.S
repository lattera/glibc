/* Copyright (C) 2002, 2003, 2004 Free Software Foundation, Inc.
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
#include <kernel-features.h>

#ifdef UP
# define LOCK
#else
# define LOCK lock
#endif

#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1
#define FUTEX_REQUEUE		3
#define FUTEX_CMP_REQUEUE	4

#define EINVAL			22


	.text

	/* int pthread_cond_broadcast (pthread_cond_t *cond) */
	.globl	__pthread_cond_broadcast
	.type	__pthread_cond_broadcast, @function
	.align	16
__pthread_cond_broadcast:

	pushl	%ebx
	pushl	%esi
	pushl	%edi
	pushl	%ebp

	movl	20(%esp), %ebx

	/* Get internal lock.  */
	movl	$1, %edx
	xorl	%eax, %eax
	LOCK
#if cond_lock == 0
	cmpxchgl %edx, (%ebx)
#else
	cmpxchgl %edx, cond_lock(%ebx)
#endif
	jnz	1f

2:	addl	$cond_futex, %ebx
	movl	total_seq+4-cond_futex(%ebx), %eax
	movl	total_seq-cond_futex(%ebx), %ebp
	cmpl	wakeup_seq+4-cond_futex(%ebx), %eax
	ja	3f
	jb	4f
	cmpl	wakeup_seq-cond_futex(%ebx), %ebp
	jna	4f

	/* Cause all currently waiting threads to recognize they are
	   woken up.  */
3:	movl	%ebp, wakeup_seq-cond_futex(%ebx)
	movl	%eax, wakeup_seq-cond_futex+4(%ebx)
	movl	%ebp, woken_seq-cond_futex(%ebx)
	movl	%eax, woken_seq-cond_futex+4(%ebx)
	addl	%ebp, %ebp
	addl	$1, broadcast_seq-cond_futex(%ebx)
	movl	%ebp, (%ebx)

	/* Get the address of the mutex used.  */
	movl	dep_mutex-cond_futex(%ebx), %edi

	/* Unlock.  */
	LOCK
	subl	$1, cond_lock-cond_futex(%ebx)
	jne	7f

	/* Don't use requeue for pshared condvars.  */
8:	cmpl	$-1, %edi
	je	9f

	/* Wake up all threads.  */
	movl	$FUTEX_CMP_REQUEUE, %ecx
	movl	$SYS_futex, %eax
	movl	$0x7fffffff, %esi
	movl	$1, %edx
	/* Get the address of the futex involved.  */
# if MUTEX_FUTEX != 0
	addl	$MUTEX_FUTEX, %edi
# endif
/* FIXME: Until Ingo fixes 4G/4G vDSO, 6 arg syscalls are broken for sysenter.
	ENTER_KERNEL  */
	int	$0x80

	/* For any kind of error, which mainly is EAGAIN, we try again
	   with WAKE.  The general test also covers running on old
	   kernels.  */
	cmpl	$0xfffff001, %eax
	jae	9f

10:	xorl	%eax, %eax
	popl	%ebp
	popl	%edi
	popl	%esi
	popl	%ebx
	ret

	.align	16
	/* Unlock.  */
4:	LOCK
	subl	$1, cond_lock-cond_futex(%ebx)
	jne	5f

6:	xorl	%eax, %eax
	popl	%ebp
	popl	%edi
	popl	%esi
	popl	%ebx
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
5:	leal	cond_lock-cond_futex(%ebx), %eax
	call	__lll_mutex_unlock_wake
	jmp	6b

	/* Unlock in loop requires waekup.  */
7:	leal	cond_lock-cond_futex(%ebx), %eax
	call	__lll_mutex_unlock_wake
	jmp	8b

9:	/* The futex requeue functionality is not available.  */
	movl	$0x7fffffff, %edx
	movl	$FUTEX_WAKE, %ecx
	movl	$SYS_futex, %eax
	ENTER_KERNEL
	jmp	10b
	.size	__pthread_cond_broadcast, .-__pthread_cond_broadcast
versioned_symbol (libpthread, __pthread_cond_broadcast, pthread_cond_broadcast,
		  GLIBC_2_3_2)
