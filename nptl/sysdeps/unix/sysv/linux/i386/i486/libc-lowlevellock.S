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
#include <tls.h>

	.text

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
#ifndef UP
	cmpl	$0, %gs:MULTIPLE_THREADS_OFFSET
	je,pt	0f
	lock
0:
#endif
	xaddl	%eax, (%ebx)
	jne,pn	1b

	movl	$-1, (%ebx)

	popl	%edx
	popl	%ebx
	popl	%esi
	ret
	.size	__lll_lock_wait,.-__lll_lock_wait


	.globl	__lll_unlock_wake
	.type	__lll_unlock_wake,@function
	.hidden	__lll_unlock_wake
	.align	16
__lll_unlock_wake:
	pushl	%ebx
	pushl	%ecx
	pushl	%edx

	movl	%eax, %ebx
1:	movl	$FUTEX_WAKE, %ecx
	movl	$1, %edx	/* Wake one thread.  */
	movl	$SYS_futex, %eax
	movl	%edx, (%ebx)	/* Stores '$1'.  */
	ENTER_KERNEL

	popl	%edx
	popl	%ecx
	popl	%ebx
	ret
	.size	__lll_unlock_wake,.-__lll_unlock_wake
