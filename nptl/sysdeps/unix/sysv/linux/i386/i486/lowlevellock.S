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

#ifdef UP
# define LOCK
#else
# define LOCK lock
#endif

#define SYS_gettimeofday	__NR_gettimeofday
#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1

#define ETIMEDOUT               110


	.globl	__lll_lock_wait
	.type	__lll_lock_wait,@function
	.hidden	__lll_lock_wait
	.align	16
__lll_lock_wait:
	pushl	%esi
	pushl	%ebx
	pushl	%edx

	movl	%ecx, %ebx
	xorl	%esi, %esi	/* No timeout.  */
	xorl	%ecx, %ecx	/* movl $FUTEX_WAIT, %ecx */
1:
	leal	-1(%eax), %edx	/* account for the preceeded xadd.  */
	movl	$SYS_futex, %eax
	ENTER_KERNEL

	orl	$-1, %eax	/* Load -1.  */
	LOCK
	xaddl	%eax, (%ebx)
	jne	1b

	movl	$-1, (%ebx)

	popl	%edx
	popl	%ebx
	popl	%esi
	ret
	.size	__lll_lock_wait,.-__lll_lock_wait


	.globl	lll_unlock_wake_cb
	.type	lll_unlock_wake_cb,@function
	.hidden	lll_unlock_wake_cb
	.align	16
lll_unlock_wake_cb:
	pushl	%esi
	pushl	%ebx
	pushl	%ecx
	pushl	%edx

	movl	20(%esp), %ebx
	LOCK
	incl	(%ebx)
	jng	1f

	popl	%edx
	popl	%ecx
	popl	%ebx
	popl	%esi
	ret
	.size	lll_unlock_wake_cb,.-lll_unlock_wake_cb


	.globl	__lll_unlock_wake
	.type	__lll_unlock_wake,@function
	.hidden	__lll_unlock_wake
__lll_unlock_wake:
	pushl	%esi
	pushl	%ebx
	pushl	%ecx
	pushl	%edx

	movl	%eax, %ebx
1:	movl	$FUTEX_WAKE, %ecx
	movl	$1, %edx	/* Wake one thread.  */
	xorl	%esi, %esi
	movl	%edx, (%ebx)	/* Stores '$1'.  */
	movl	$SYS_futex, %eax
	ENTER_KERNEL

	popl	%edx
	popl	%ecx
	popl	%ebx
	popl	%esi
	ret
	.size	__lll_unlock_wake,.-__lll_unlock_wake


	.globl	__lll_timedwait_tid
	.type	__lll_timedwait_tid,@function
	.hidden	__lll_timedwait_tid
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
	decl	%ecx
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

	movl	%eax, %edx

	cmpl	$0, (%ebx)
	jne	1f
4:	xorl	%eax, %eax

3:	addl	$8, %esp
	popl	%ebp
	popl	%ebx
	popl	%esi
	popl	%edi
	ret

1:	cmpl	$-ETIMEDOUT, %edx
	jne	2b
6:	movl	$ETIMEDOUT, %eax
	jmp	3b
	.size	__lll_timedwait_tid,.-__lll_timedwait_tid
