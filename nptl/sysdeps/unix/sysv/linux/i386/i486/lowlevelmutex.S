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
#include <pthread-errnos.h>

	.text

#ifndef LOCK
# ifdef UP
#  define LOCK
# else
#  define LOCK lock
# endif
#endif

#define SYS_gettimeofday	__NR_gettimeofday
#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1


	.globl	__lll_mutex_lock_wait
	.type	__lll_mutex_lock_wait,@function
	.hidden	__lll_mutex_lock_wait
	.align	16
__lll_mutex_lock_wait:
	pushl	%esi
	pushl	%ebx
	pushl	%edx

	movl	%ecx, %ebx
	xorl	%esi, %esi	/* No timeout.  */
	xorl	%ecx, %ecx	/* movl $FUTEX_WAIT, %ecx */
1:
	leal	1(%eax), %edx	/* account for the preceeded xadd.  */
	movl	$SYS_futex, %eax
	ENTER_KERNEL

	movl	$1, %eax
	LOCK
	xaddl	%eax, (%ebx)
	testl	%eax, %eax
	jne	1b

	movl	$2, (%ebx)

	popl	%edx
	popl	%ebx
	popl	%esi
	ret
	.size	__lll_mutex_lock_wait,.-__lll_mutex_lock_wait


#ifdef NOT_IN_libc
	.globl	__lll_mutex_timedlock_wait
	.type	__lll_mutex_timedlock_wait,@function
	.hidden	__lll_mutex_timedlock_wait
	.align	16
__lll_mutex_timedlock_wait:
	/* Check for a valid timeout value.  */
	cmpl	$1000000000, 4(%edx)
	jae	3f

	pushl	%edi
	pushl	%esi
	pushl	%ebx
	pushl	%ebp

	/* Stack frame for the timespec and timeval structs.  */
	subl	$8, %esp

	movl	%ecx, %ebp
	movl	%edx, %edi
	leal	1(%eax), %esi

	/* Get current time.  */
1:
	movl	%esp, %ebx
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
	jns	4f
	addl	$1000000000, %edx
	subl	$1, %ecx
4:	testl	%ecx, %ecx
	js	5f		/* Time is already up.  */

	/* Futex call.  */
	movl	%ecx, (%esp)	/* Store relative timeout.  */
	movl	%edx, 4(%esp)
	movl	%esi, %edx
	movl	%esp, %esi
	xorl	%ecx, %ecx	/* movl $FUTEX_WAIT, %ecx */
	movl	%ebp, %ebx
	movl	$SYS_futex, %eax
	ENTER_KERNEL

	movl	$1, %esi
	LOCK
	xaddl	%esi, (%ebx)
	testl	%esi, %esi
	jne	7f

	movl	$2, (%ebx)
	xorl	%eax, %eax

6:	addl	$8, %esp
	popl	%ebp
	popl	%ebx
	popl	%esi
	popl	%edi
	ret

	/* Check whether the time expired.  */
7:	cmpl	$-ETIMEDOUT, %eax
	je	5f
	jmp	1b

3:	movl	$EINVAL, %eax
	ret

5:	movl	$ETIMEDOUT, %eax
	jmp	6b
	.size	__lll_mutex_timedlock_wait,.-__lll_mutex_timedlock_wait
#endif


	.globl	__lll_mutex_unlock_wake
	.type	__lll_mutex_unlock_wake,@function
	.hidden	__lll_mutex_unlock_wake
	.align	16
__lll_mutex_unlock_wake:
	pushl	%ebx
	pushl	%ecx
	pushl	%edx

	movl	%eax, %ebx
	movl	$0, (%eax)
	movl	$FUTEX_WAKE, %ecx
	movl	$1, %edx	/* Wake one thread.  */
	movl	$SYS_futex, %eax
	ENTER_KERNEL

	popl	%edx
	popl	%ecx
	popl	%ebx
	ret
	.size	__lll_mutex_unlock_wake,.-__lll_mutex_unlock_wake
