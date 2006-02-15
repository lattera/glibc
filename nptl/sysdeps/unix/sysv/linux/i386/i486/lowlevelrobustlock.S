/* Copyright (C) 2002, 2003, 2004, 2006 Free Software Foundation, Inc.
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
#include <lowlevelrobustlock.h>

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
#define FUTEX_WAITERS		0x80000000
#define FUTEX_OWNER_DIED	0x40000000


	.globl	__lll_robust_mutex_lock_wait
	.type	__lll_robust_mutex_lock_wait,@function
	.hidden	__lll_robust_mutex_lock_wait
	.align	16
__lll_robust_mutex_lock_wait:
	pushl	%edx
	pushl	%ebx
	pushl	%esi

	movl	%ecx, %ebx
	xorl	%esi, %esi	/* No timeout.  */
	xorl	%ecx, %ecx	/* movl $FUTEX_WAIT, %ecx */

4:	movl	%eax, %edx
	orl	$FUTEX_WAITERS, %edx

	testl	$FUTEX_OWNER_DIED, %eax
	jnz	3f

	cmpl	%edx, %eax	/* NB:	 %edx == 2 */
	je	1f

	LOCK
	cmpxchgl %edx, (%ebx)
	jnz	2f

1:	movl	$SYS_futex, %eax
	ENTER_KERNEL

	movl	(%ebx), %eax

2:	test	%eax, %eax
	jne	4b

	movl	%gs:TID, %edx
	LOCK
	cmpxchgl %edx, (%ebx)
	jnz	4b
	/* NB:	 %eax == 0 */

3:	popl	%esi
	popl	%ebx
	popl	%edx
	ret
	.size	__lll_robust_mutex_lock_wait,.-__lll_robust_mutex_lock_wait


	.globl	__lll_robust_mutex_timedlock_wait
	.type	__lll_robust_mutex_timedlock_wait,@function
	.hidden	__lll_robust_mutex_timedlock_wait
	.align	16
__lll_robust_mutex_timedlock_wait:
	/* Check for a valid timeout value.  */
	cmpl	$1000000000, 4(%edx)
	jae	3f

	pushl	%edi
	pushl	%esi
	pushl	%ebx
	pushl	%ebp

	/* Stack frame for the timespec and timeval structs.  */
	subl	$12, %esp

	movl	%ecx, %ebp
	movl	%edx, %edi

1:	movl	%eax, 8(%esp)

	/* Get current time.  */
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
	js	8f		/* Time is already up.  */

	/* Store relative timeout.  */
	movl	%ecx, (%esp)
	movl	%edx, 4(%esp)

	movl	%ebp, %ebx

	movl	8(%esp), %edx
	movl	%edx, %eax
	orl	$FUTEX_WAITERS, %edx

	testl	$FUTEX_OWNER_DIED, %eax
	jnz	6f

	cmpl	%eax, %edx
	je	2f

	LOCK
	cmpxchgl %edx, (%ebx)
	movl	$0, %ecx	/* Must use mov to avoid changing cc.  */
	jnz	5f

2:
	/* Futex call.  */
	movl	%esp, %esi
	xorl	%ecx, %ecx	/* movl $FUTEX_WAIT, %ecx */
	movl	$SYS_futex, %eax
	ENTER_KERNEL
	movl	%eax, %ecx

	movl	(%ebx), %eax

5:	testl	%eax, %eax
	jne	7f

	movl	%gs:TID, %edx
	LOCK
	cmpxchgl %edx, (%ebx)
	jnz	7f

6:	addl	$12, %esp
	popl	%ebp
	popl	%ebx
	popl	%esi
	popl	%edi
	ret

	/* Check whether the time expired.  */
7:	cmpl	$-ETIMEDOUT, %ecx
	jne	1b

8:	movl	$ETIMEDOUT, %eax
	jmp	6b

3:	movl	$EINVAL, %eax
	ret
	.size	__lll_robust_mutex_timedlock_wait,.-__lll_robust_mutex_timedlock_wait
