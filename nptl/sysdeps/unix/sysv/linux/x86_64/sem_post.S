/* Copyright (C) 2002,2003,2005,2007,2008,2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <lowlevellock.h>
#include <shlib-compat.h>
#include <pthread-errnos.h>
#include <structsem.h>


	.text

	.globl	sem_post
	.type	sem_post,@function
	.align	16
sem_post:
#if VALUE == 0
	movl	(%rdi), %eax
#else
	movl	VALUE(%rdi), %eax
#endif
0:	cmpl	$SEM_VALUE_MAX, %eax
	je	3f
	leal	1(%rax), %esi
	LOCK
#if VALUE == 0
	cmpxchgl %esi, (%rdi)
#else
	cmpxchgl %esi, VALUE(%rdi)
#endif
	jnz	0b

	LP_OP(cmp) $0, NWAITERS(%rdi)
	je	2f

	movl	$SYS_futex, %eax
	movl	$FUTEX_WAKE, %esi
	orl	PRIVATE(%rdi), %esi
	movl	$1, %edx
	syscall

	testq	%rax, %rax
	js	1f

2:	xorl	%eax, %eax
	retq

1:
	movl	$EINVAL, %eax
	jmp	4f

3:
	movl	$EOVERFLOW, %eax

4:
	movq	errno@gottpoff(%rip), %rdx
	movl	%eax, %fs:(%rdx)
	orl	$-1, %eax
	retq
	.size	sem_post,.-sem_post
