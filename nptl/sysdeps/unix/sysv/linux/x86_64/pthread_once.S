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

#ifndef UP
# define LOCK lock
#else
# define LOCK
#endif

#define SYS_futex	202
#define FUTEX_WAKE	1

	.comm	__fork_generation, 4, 4

	.text


	.globl	__pthread_once
	.type	__pthread_once,@function
	.align	16
__pthread_once:
	testl	$2, (%rdi)
	jz	1f
	xorl	%eax, %eax
	retq

	/* Preserve the function pointer.  */
1:	pushq	%rsi
	xorq	%r10, %r10

	/* Not yet initialized or initialization in progress.
	   Get the fork generation counter now.  */
6:	movl	(%rdi), %eax

5:	movl	%eax, %edx

	testl	$2, %eax
	jnz	4f

	andl	$3, %edx
	orl	__fork_generation(%rip), %edx
	orl	$1, %edx

	LOCK
	cmpxchgl %edx, (%rdi)
	jnz	5b

	/* Check whether another thread already runs the initializer.  */
	testl	$1, %eax
	jz	3f	/* No -> do it.  */

	/* Check whether the initializer execution was interrupted
	   by a fork.  */
	xorl	%edx, %eax
	testl	$0xfffffffc, %eax
	jnz	3f	/* Different for generation -> run initializer.  */

	/* Somebody else got here first.  Wait.  */
	movq	%r10, %rsi		/* movl $FUTEX_WAIT, %ecx */
	movq	$SYS_futex, %rax
	syscall
	jmp	6b

	/* Preserve the pointer to the control variable.  */
3:	pushq	%rdi

	/* Call the initializer function after setting up the
	   cancellation handler.  */
	subq	$32, %rsp

	/* Push the cleanup handler.  */
	leaq	clear_once_control(%rip), %rsi
	movq	%rdi, %rdx
	movq	%rsp, %rdi
	callq	__pthread_cleanup_push	/* Note: no @PLT.  */

	callq	*40(%rsp)

	/* Pop the cleanup handler.  */
	movq	%rsp, %rdi
	xorq	%rsi, %rsi
	callq	__pthread_cleanup_pop	/* Note: no @PLT.  */

	addq	$32, %rsp

	/* Get the control variable address back.  */
	popq	%rdi

	/* Sucessful run of the initializer.  Signal that we are done.  */
	LOCK
	incl	(%rdi)

	/* Wake up all other threads.  */
	movl	$0x7fffffff, %edx
	movl	$FUTEX_WAKE, %esi
	movq	$SYS_futex, %rax
	syscall

4:	addq	$4, %rsp
	xorq	%rax, %rax
	retq

	.size	__pthread_once,.-__pthread_once


	.globl	__pthread_once_internal
__pthread_once_internal = __pthread_once

	.globl	pthread_once
pthread_once = __pthread_once


	.type	clear_once_control,@function
	.align	16
clear_once_control:
	movl	$0, (%rdi)

	movl	$0x7fffffff, %edx
	movl	$FUTEX_WAKE, %esi
	movq	$SYS_futex, %rax
	syscall

	retq
	.size	clear_once_control,.-clear_once_control
