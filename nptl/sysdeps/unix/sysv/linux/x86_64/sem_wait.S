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
	cfi_startproc
sem_wait:
	/* First check for cancellation.  */
	movl	%fs:CANCELHANDLING, %eax
	andl	$0xfffffff9, %eax
	cmpl	$8, %eax
	je	4f

	pushq	%r12
	cfi_adjust_cfa_offset(8)
	cfi_offset(12, -16)
	pushq	%r13
	cfi_adjust_cfa_offset(8)
	movq	%rdi, %r13
	cfi_offset(13, -24)

3:	movl	(%r13), %eax
2:	testl	%eax, %eax
	je	1f

	leaq	-1(%rax), %rdx
	LOCK
	cmpxchgl %edx, (%r13)
	jne	2b
	xorl	%eax, %eax

	popq	%r13
	cfi_adjust_cfa_offset(-8)
	cfi_restore(13)
	popq	%r12
	cfi_adjust_cfa_offset(-8)
	cfi_restore(12)

	retq

	cfi_adjust_cfa_offset(16)
	cfi_offset(12, -16)
	cfi_offset(13, -24)
1:	call	__pthread_enable_asynccancel
	movl	%eax, %r8d

	xorq	%r10, %r10
	movq	$SYS_futex, %rax
	movq	%r13, %rdi
	movq	%r10, %rsi
	movq	%r10, %rdx
	syscall
	movq	%rax, %r12

	movl	%r8d, %edi
	call	__pthread_disable_asynccancel

	testq	%r12, %r12
	je	3b
	cmpq	$-EWOULDBLOCK, %r12
	je	3b
	negq	%r12
#if USE___THREAD
	movq	errno@gottpoff(%rip), %rdx
	movl	%r12d, %fs:(%rdx)
#else
	callq	__errno_location@plt
	movl	%r12d, (%rax)
#endif
	orl	$-1, %eax

	popq	%r13
	cfi_adjust_cfa_offset(-8)
	cfi_restore(13)
	popq	%r12
	cfi_adjust_cfa_offset(-8)
	cfi_restore(12)

	retq

4:	/* Canceled.  */
	movq	$0xffffffffffffffff, %fs:RESULT
	LOCK
	orl	$0x10, %fs:CANCELHANDLING
	movq	%fs:CLEANUP_JMP_BUF, %rdi
	jmp	HIDDEN_JUMPTARGET (__pthread_unwind)
	cfi_endproc
	.size	sem_wait,.-sem_wait
