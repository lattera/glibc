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


	.text

	.globl	sem_wait
	.type	sem_wait,@function
	.align	16
sem_wait:
3:	movl	(%rdi), %eax
2:	testl	%eax, %eax
	je	1f

	leaq	-1(%rax), %rdx
	LOCK
	cmpxchgl %edx, (%rdi)
	jne	2b
	xorl	%eax, %eax

	retq

1:	xorq	%r10, %r10
	movq	$SYS_futex, %rax
	movq	%r10, %rsi
	movq	%r10, %rdx
	syscall

	testq	%rax, %rax
	je	3b
	cmpq	$-EWOULDBLOCK, %rax
	je	3b
	negq	%rax
#if USE___THREAD
	movq	errno@gottpoff(%rip), %rdx
	movl	%eax, %fs:(%rdx)
#else
	movl	%eax, %edx
	callq	__errno_location@plt
	movl	%edx, (%rax)
#endif
	orl	$-1, %eax
	retq
	.size	sem_wait,.-sem_wait
