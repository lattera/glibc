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

#define EINVAL			22


	.text

	/* int pthread_cond_signal (pthread_cond_t *cond) */
	.globl	__pthread_cond_signal
	.type	__pthread_cond_signal, @function
	.align	16
__pthread_cond_signal:

	pushl	%ebx
	pushl	%edi

	movl	12(%esp), %edi

	/* Get internal lock.  */
	movl	$1, %edx
	xorl	%eax, %eax
	LOCK
#if cond_lock == 0
	cmpxchgl %edx, (%edi)
#else
	cmpxchgl %edx, cond_lock(%edi)
#endif
	jnz	1f

2:	leal	cond_futex(%edi), %ebx
	movl	total_seq+4(%edi), %eax
	movl	total_seq(%edi), %ecx
	cmpl	wakeup_seq+4(%edi), %eax
#if cond_lock != 0
	/* Must use leal to preserve the flags.  */
	leal	cond_lock(%edi), %edi
#endif
	ja	3f
	jb	4f
	cmpl	wakeup_seq-cond_futex(%ebx), %ecx
	jbe	4f

	/* Bump the wakeup number.  */
3:	addl	$1, wakeup_seq-cond_futex(%ebx)
	adcl	$0, wakeup_seq-cond_futex+4(%ebx)
	addl	$1, (%ebx)

	/* Wake up one thread.  */
	movl	$FUTEX_WAKE, %ecx
	movl	$SYS_futex, %eax
	movl	$1, %edx
	ENTER_KERNEL

	/* Unlock.  Note that at this point %edi always points to
	   cond_lock.  */
4:	LOCK
	subl	$1, (%edi)
	jne	5f

6:	xorl	%eax, %eax
	popl	%edi
	popl	%ebx
	ret

	/* Initial locking failed.  */
1:
#if cond_lock == 0
	movl	%edi, %ecx
#else
	leal	cond_lock(%edi), %ecx
#endif
	call	__lll_mutex_lock_wait
	jmp	2b

	/* Unlock in loop requires wakeup.  */
5:	movl	%edi, %eax
	call	__lll_mutex_unlock_wake
	jmp	6b
	.size	__pthread_cond_signal, .-__pthread_cond_signal
versioned_symbol (libpthread, __pthread_cond_signal, pthread_cond_signal,
		  GLIBC_2_3_2)
