/* Install given context.
   Copyright (C) 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2001.

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

#include "ucontext_i.h"


ENTRY(__setcontext)
	/* Load address of the context data structure.  */
	movl	4(%esp), %eax

	/* Get the current signal mask.  Note that we preserve EBX in case
	   the system call fails and we return from the function with an
	   error.  */
	pushl	%ebx
	xorl	%edx, %edx
	leal	oSIGMASK(%eax), %ecx
	movl	$SIG_SETMASK, %ebx
	movl	$__NR_sigprocmask, %eax
	int	$0x80
	popl	%ebx
	cmpl	$-4095, %eax		/* Check %eax for error.  */
	jae	SYSCALL_ERROR_LABEL	/* Jump to error handler if error.  */

	/* EAX was modified, reload it.  */
	movl	4(%esp), %eax

	/* Restore the floating-point context.  Not the registers, only the
	   rest.  */
	movl	oFPREGS(%eax), %ecx
	fldenv	(%ecx)

	/* Restore the FS and GS segment registers.  */
	movl	oGS(%eax), %edx
	movl	oFS(%eax), %ecx
	movw	%dx, %gs
	movw	%cx, %fs

	/* Load the new stack pointer.  */
	movl	oESP(%eax), %ecx
	/* Make room for 8 registers and the return address.  We will load
	   the values from the stack.  */
	subl	$36, %ecx

	/* Move the values of all the 32-bit registers (except ESP) on
	   the stack.  This happens in the form the 'popa' instruction
	   expects it.  Before this block put the address of the code
	   to execute.  */
	movl	oEDI(%eax), %ebx
	movl	oESI(%eax), %edx
	movl	oEBP(%eax), %esi
	movl	oEBX(%eax), %edi
	movl	%ebx, (%ecx)
	movl	%edx, 4(%ecx)
	movl	%esi, 8(%ecx)
	movl	%edi, 16(%ecx)
	movl	oEDX(%eax), %ebx
	movl	oECX(%eax), %edx
	movl	oEAX(%eax), %esi
	movl	oEIP(%eax), %edi
	movl	%ebx, 20(%ecx)
	movl	%edx, 24(%ecx)
	movl	%esi, 28(%ecx)
	movl	%edi, 32(%ecx)

	/* Set the new stack address.  The stack points now to the block
	   we put the register content in.  */
	movl	%ecx, %esp
	/* Restore the register content.  */
	popa
	/* The following 'ret' will pop the addres of the code and jump
	   to it.  */

L(pseudo_end):
	ret
PSEUDO_END(__setcontext)

weak_alias(__setcontext, setcontext)
