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
	pushl	%edx
	pushl	%ebx
	pushl	%esi

	movl	$2, %edx
	movl	%ecx, %ebx
	xorl	%esi, %esi	/* No timeout.  */
	xorl	%ecx, %ecx	/* movl $FUTEX_WAIT, %ecx */

	cmpl	%edx, %eax	/* NB:	 %edx == 2 */
	jne 2f

1:	movl	$SYS_futex, %eax
	ENTER_KERNEL

2:	movl	%edx, %eax
	xchgl	%eax, (%ebx)	/* NB:	 lock is implied */

	testl	%eax, %eax
	jnz,pn	1b

	popl	%esi
	popl	%ebx
	popl	%edx
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

1:
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
	js	5f		/* Time is already up.  */

	/* Store relative timeout.  */
	movl	%ecx, (%esp)
	movl	%edx, 4(%esp)

	movl	%ebp, %ebx

	movl	$1, %eax
	movl	$2, %edx
	LOCK
	cmpxchgl %edx, (%ebx)

	testl	%eax, %eax
	je	8f

	/* Futex call.  */
	movl	%esp, %esi
	xorl	%ecx, %ecx	/* movl $FUTEX_WAIT, %ecx */
	movl	$SYS_futex, %eax
	ENTER_KERNEL
	movl	%eax, %ecx

8:				/* NB: %edx == 2 */
	xorl	%eax, %eax
	LOCK
	cmpxchgl %edx, (%ebx)

	jnz	7f

6:	addl	$8, %esp
	popl	%ebp
	popl	%ebx
	popl	%esi
	popl	%edi
	ret

	/* Check whether the time expired.  */
7:	cmpl	$-ETIMEDOUT, %ecx
	je	5f

	/* Make sure the current holder knows we are going to sleep.  */
	movl	%edx, %eax
	xchgl	%eax, (%ebx)
	testl	%eax, %eax
	jz	6b
	jmp	1b

3:	movl	$EINVAL, %eax
	ret

5:	movl	$ETIMEDOUT, %eax
	jmp	6b
	.size	__lll_mutex_timedlock_wait,.-__lll_mutex_timedlock_wait
#endif


#ifdef NOT_IN_libc
	.globl	lll_unlock_wake_cb
	.type	lll_unlock_wake_cb,@function
	.hidden	lll_unlock_wake_cb
	.align	16
lll_unlock_wake_cb:
	pushl	%ebx
	pushl	%ecx
	pushl	%edx

	movl	20(%esp), %ebx
	LOCK
	subl	$1, (%ebx)
	je	1f

	movl	$FUTEX_WAKE, %ecx
	movl	$1, %edx	/* Wake one thread.  */
	movl	$SYS_futex, %eax
	movl	$0, (%ebx)
	ENTER_KERNEL

1:	popl	%edx
	popl	%ecx
	popl	%ebx
	ret
	.size	lll_unlock_wake_cb,.-lll_unlock_wake_cb
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


#ifdef NOT_IN_libc
	.globl	__lll_timedwait_tid
	.type	__lll_timedwait_tid,@function
	.hidden	__lll_timedwait_tid
	.align	16
__lll_timedwait_tid:
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	pushl	%ebp

	movl	%eax, %ebp
	movl	%edx, %edi
	subl	$8, %esp

	/* Get current time.  */
2:	movl	%esp, %ebx
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
	jns	5f
	addl	$1000000000, %edx
	subl	$1, %ecx
5:	testl	%ecx, %ecx
	js	6f		/* Time is already up.  */

	movl	%ecx, (%esp)	/* Store relative timeout.  */
	movl	%edx, 4(%esp)

	movl	(%ebp), %edx
	testl	%edx, %edx
	jz	4f

	movl	%esp, %esi
	xorl	%ecx, %ecx	/* movl $FUTEX_WAIT, %ecx */
	movl	%ebp, %ebx
	movl	$SYS_futex, %eax
	ENTER_KERNEL

	cmpl	$0, (%ebx)
	jne	1f
4:	xorl	%eax, %eax

3:	addl	$8, %esp
	popl	%ebp
	popl	%ebx
	popl	%esi
	popl	%edi
	ret

1:	cmpl	$-ETIMEDOUT, %eax
	jne	2b
6:	movl	$ETIMEDOUT, %eax
	jmp	3b
	.size	__lll_timedwait_tid,.-__lll_timedwait_tid
#endif
