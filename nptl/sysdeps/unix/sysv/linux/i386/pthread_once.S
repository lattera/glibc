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

#ifndef UP
# define LOCK lock
#else
# define LOCK
#endif

#define SYS_futex	240
#define FUTEX_WAKE	1

	.comm	__fork_generation, 4, 4

	.text


	.globl	__pthread_once
	.type	__pthread_once,@function
	.align	16
__pthread_once:
	movl	4(%esp), %ecx
	testl	$2, (%ecx)
	jz	1f
	xorl	%eax, %eax
	ret

1:	pushl	%ebx
	pushl	%esi
	movl	%ecx, %ebx
	xorl	%esi, %esi

	/* Not yet initialized or initialization in progress.
	   Get the fork generation counter now.  */
6:	movl	(%ebx), %eax
#ifdef PIC
	call	__i686.get_pc_thunk.cx
	addl	$_GLOBAL_OFFSET_TABLE_, %ecx
#endif

5:	movl	%eax, %edx

	testl	$2, %eax
	jnz	4f

	andl	$3, %edx
#ifdef PIC
	orl	__fork_generation@GOTOFF(%ecx), %edx
#else
	orl	__fork_generation, %edx
#endif
	orl	$1, %edx

	LOCK
	cmpxchgl %edx, (%ebx)
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
	movl	%esi, %ecx		/* movl $FUTEX_WAIT, %ecx */
	movl	$SYS_futex, %eax
	int	$0x80
	jmp	6b

3:	/* Call the initializer function after setting up the
	   cancellation handler.  */
	subl	$16, %esp

	/* Push the cleanup handler.  */
#ifdef PIC
	leal	clear_once_control@GOTOFF(%ecx), %eax
#else
	leal	clear_once_control, %eax
#endif
	movl	%esp, %edx
	pushl	%ebx
	pushl	%eax
	pushl	%edx
	call	_GI_pthread_cleanup_push	/* Note: no @PLT.  */

	movl	44(%esp), %eax
	call	*%eax

	/* Pop the cleanup handler.  This code depends on the once
	   handler and _pthread_cleanup_push not touch the content
	   of the stack.  Otherwise the first parameter would have
	   to be reloaded.  */
	movl	$0, 4(%esp)
	call	_GI_pthread_cleanup_pop	/* Note: no @PLT.  */

	addl	$28, %esp

	/* Sucessful run of the initializer.  Signal that we are done.  */
	LOCK
	incl	(%ebx)

	/* Wake up all other threads.  */
	movl	$0x7fffffff, %edx
	movl	$FUTEX_WAKE, %ecx
	movl	$SYS_futex, %eax
	int	$0x80

4:	popl	%esi
	popl	%ebx
	xorl	%eax, %eax
	ret

	.size	__pthread_once,.-__pthread_once

	.globl	pthread_once
pthread_once = __pthread_once


	.type	clear_once_control,@function
	.align	16
clear_once_control:
	pushl	%esi
	pushl	%ebx

	movl	4(%esp), %eax
	movl	$0, (%eax)

	xorl	%esi, %esi
	movl	$0x7fffffff, %edx
	movl	$FUTEX_WAKE, %ecx
	movl	$SYS_futex, %eax
	int	$0x80

	popl	%ebx
	popl	%esi
	ret
	.size	clear_once_control,.-clear_once_control


#ifdef PIC
	.section .gnu.linkonce.t.__i686.get_pc_thunk.cx,"ax",@progbits
	.globl	__i686.get_pc_thunk.cx
	.hidden	__i686.get_pc_thunk.cx
	.type	__i686.get_pc_thunk.cx,@function
__i686.get_pc_thunk.cx:
	movl (%esp), %ecx;
	ret
	.size	__i686.get_pc_thunk.cx,.-__i686.get_pc_thunk.cx
#endif
