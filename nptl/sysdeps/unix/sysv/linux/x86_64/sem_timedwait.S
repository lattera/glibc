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
	cfi_startproc
sem_timedwait:
	/* First check for cancellation.  */
	movl	%fs:CANCELHANDLING, %eax
	andl	$0xfffffff9, %eax
	cmpl	$8, %eax
	je	11f

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
	cfi_adjust_cfa_offset(8)
	pushq	%r13
	cfi_adjust_cfa_offset(8)
	pushq	%r14
	cfi_adjust_cfa_offset(8)
	subq	$24, %rsp
	cfi_adjust_cfa_offset(24)

	movq	%rdi, %r12
	cfi_offset(12, -16)		/* %r12 */
	movq	%rsi, %r13
	cfi_offset(13, -24)		/* %r13 */

	/* Check for invalid nanosecond field.  */
	cmpq	$1000000000, 8(%r13)
	movl	$EINVAL, %r14d
	cfi_offset(14, -24)		/* %r14 */
	jae	6f

7:	call	__pthread_enable_asynccancel
	movl	%eax, 16(%rsp)

	xorq	%rsi, %rsi
	movq	%rsp, %rdi
	movq	$VSYSCALL_ADDR_vgettimeofday, %rax
	callq	*%rax

	/* Compute relative timeout.  */
	movq	8(%rsp), %rax
	movq	$1000, %rdi
	mul	%rdi		/* Milli seconds to nano seconds.  */
	movq	(%r13), %rdi
	movq	8(%r13), %rsi
	subq	(%rsp), %rdi
	subq	%rax, %rsi
	jns	5f
	addq	$1000000000, %rsi
	decq	%rdi
5:	testq	%rdi, %rdi
	movl	$ETIMEDOUT, %r14d
	js	6f		/* Time is already up.  */

	movq	%rdi, (%rsp)	/* Store relative timeout.  */
	movq	%rsi, 8(%rsp)

	movq	%rsp, %r10
	movq	%r12, %rdi
	xorq	%rsi, %rsi
	movq	$SYS_futex, %rax
	xorl	%edx, %edx
	syscall
	movq	%rax, %r14

	movl	16(%rsp), %edi
	call	__pthread_disable_asynccancel

	testq	%r14, %r14
	je	9f
	cmpq	$-EWOULDBLOCK, %r14
	jne	3f

9:	movl	(%r12), %eax
8:	testl	%eax, %eax
	je	7b

	leaq	-1(%rax), %rcx
	LOCK
	cmpxchgl %ecx, (%r12)
	jne	8b

	xorl	%eax, %eax
10:	addq	$24, %rsp
	cfi_adjust_cfa_offset(-24)
	popq	%r14
	cfi_adjust_cfa_offset(-8)
	cfi_restore(14)
	popq	%r13
	cfi_adjust_cfa_offset(-8)
	cfi_restore(13)
	popq	%r12
	cfi_adjust_cfa_offset(-8)
	cfi_restore(12)
	retq

	cfi_adjust_cfa_offset(48)
	cfi_offset(12, -16)		/* %r12 */
	cfi_offset(13, -24)		/* %r13 */
	cfi_offset(14, -32)		/* %r14 */
3:	negq	%r14
6:
#if USE___THREAD
	movq	errno@gottpoff(%rip), %rdx
	movl	%r14d, %fs:(%rdx)
#else
	callq	__errno_location@plt
	movl	%r14d, (%rax)
#endif

	orl	$-1, %eax
	jmp	10b
	cfi_adjust_cfa_offset(-48)
	cfi_restore(14)
	cfi_restore(13)
	cfi_restore(12)

11:	/* Canceled.  */
	movq	$0xffffffffffffffff, %fs:RESULT
	LOCK
	orl	$0x10, %fs:CANCELHANDLING
	movq	%fs:CLEANUP_JMP_BUF, %rdi
	jmp	HIDDEN_JUMPTARGET (__pthread_unwind)
	cfi_endproc
	.size	sem_timedwait,.-sem_timedwait
