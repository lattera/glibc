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

#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1


	.text

	/* int pthread_cond_broadcast (pthread_cond_t *cond) */
	.globl	__pthread_cond_broadcast
	.type	__pthread_cond_broadcast, @function
	.align	16
__pthread_cond_broadcast:

	pushl	%esi
	pushl	%ebx

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

2:	addl	$wakeup_seq, %ebx
	movl	total_seq+4-wakeup_seq(%ebx), %eax
	movl	total_seq-wakeup_seq(%ebx), %ecx
	cmpl	4(%ebx), %eax
	ja	3f
	jb	4f
	cmpl	(%ebx), %ecx
	jna	4f

	/* Case all currently waiting threads to wake up.  */
3:	movl	%ecx, (%ebx)
	movl	%eax, 4(%ebx)

	/* Wake up all threads.  */
	movl	$FUTEX_WAKE, %ecx
	xorl	%esi, %esi
	movl	$SYS_futex, %eax
	movl	$0x7fffffff, %edx
	ENTER_KERNEL

	/* Unlock.  */
4:	LOCK
	decl	cond_lock-wakeup_seq(%ebx)
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
	leal	cond_lock-wakeup_seq(%ebx), %eax
	call	__lll_mutex_unlock_wake
	jmp	6b
	.size	__pthread_cond_broadcast, .-__pthread_cond_broadcast
versioned_symbol (libpthread, __pthread_cond_broadcast, pthread_cond_broadcast,
		  GLIBC_2_3_2)
