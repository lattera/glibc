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
#define FUTEX_WAKE		1


	.text

	.globl	sem_post
	.type	sem_post,@function
	.align	16
sem_post:
	movl	$1, %edx
	LOCK
	xaddl	%edx, (%rdi)

	movq	$SYS_futex, %rax
	movq	$FUTEX_WAKE, %rsi
	incl	%edx
	syscall

	testq	%rax, %rax
	js	1f

	xorl	%eax, %eax
	retq

1:
#if USE___THREAD
	movq	errno@gottpoff(%rip), %rdx
	movl	$EINVAL, %fs:(%rdx)
#else
	callq	__errno_location@plt
	movl	$EINVAL, (%rax)
#endif

	orl	$-1, %eax
	retq
	.size	sem_post,.-sem_post
