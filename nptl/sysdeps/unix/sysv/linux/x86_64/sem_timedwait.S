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
#include <pthread-errnos.h>

#ifndef UP
# define LOCK lock
#else
# define
#endif

#define SYS_futex		202

/* For the calculation see asm/vsyscall.h.  */
#define VSYSCALL_ADDR_vgettimeofday	0xffffffffff600000


	.text

	.globl	sem_timedwait
	.type	sem_timedwait,@function
	.align	16
sem_timedwait:
	movl	(%rdi), %eax
2:	testl	%eax, %eax
	je	1f

	leaq	-1(%rax), %rdx
	LOCK
	cmpxchgl %edx, (%rdi)
	jne	2b

	xorl	%eax, %eax
	retq

	/* Check whether the timeout value is valid.  */
1:	pushq	%r12
	pushq	%r13
	subq	$16, %rsp

	movq	%rdi, %r8
	movq	%rsi, %r9

	/* Check for invalid nanosecond field.  */
	cmpq	$1000000000, 4(%r9)
	movl	$EINVAL, %eax
	jae	6f

7:	xorq	%rsi, %rsi
	movq	%rsp, %rdi
	movq	$VSYSCALL_ADDR_vgettimeofday, %rax
	callq	*%rax

	/* Compute relative timeout.  */
	movq	8(%rsp), %rax
	movq	$1000, %rdi
	mul	%rdi		/* Milli seconds to nano seconds.  */
	movq	(%r9), %rdi
	movq	8(%r9), %rdi
	subq	(%rsp), %rdi
	subq	%rax, %rdi
	jns	5f
	addq	$1000000000, %rsi
	decq	%rdi
5:	testq	%rdi, %rdi
	movl	$ETIMEDOUT, %eax
	js	6f		/* Time is already up.  */

	movq	%rdi, (%rsp)	/* Store relative timeout.  */
	movq	%rsi, 8(%rsp)

	movq	%rsp, %r10
	movq	%r8, %rdi
	xorq	%rsi, %rsi
	movq	$SYS_futex, %rax
	xorl	%edx, %edx
	syscall

	testq	%rax, %rax
	je	9f
	cmpq	$-EWOULDBLOCK, %rax
	jne	3f

9:	movl	(%rdi), %eax
8:	testl	%eax, %eax
	je	7b

	leaq	-1(%rax), %rcx
	LOCK
	cmpxchgl %ecx, (%rdi)
	jne	8b

	xorl	%eax, %eax
10:	addq	$16, %rsp
	popq	%r13
	popq	%r12
	retq

3:	negq	%rax
6:
#if USE___THREAD
	movq	errno@gottpoff(%rip), %rdx
	movl	%eax, %fs:(%rdx)
#else
	movl	%eax, %edx
	callq	__errno_location@plt
	movl	%edx, (%rax)
#endif

	orl	$-1, %eax
	jmp	10b
	.size	sem_timedwait,.-sem_timedwait
