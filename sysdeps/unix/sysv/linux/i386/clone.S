/* Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson (rth@tamu.edu)

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>
#include <asm-syntax.h>

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg); */

        .text
ENTRY(__clone)
	/* Sanity check arguments.  */
	movl	$-EINVAL,%eax
	movl	4(%esp),%ecx		/* no NULL function pointers */
	testl	%ecx,%ecx
	jz	SYSCALL_ERROR_LABEL
	movl	8(%esp),%ecx		/* no NULL stack pointers */
	testl	%ecx,%ecx
	jz	SYSCALL_ERROR_LABEL

	/* Insert the argument onto the new stack.  */
	subl	$8,%ecx
	movl	16(%esp),%eax		/* no negative argument counts */
	movl	%eax,4(%ecx)

	/* Save the function pointer as the zeroth argument.
	   It will be popped off in the child in the ebx frobbing below.  */
	movl	4(%esp),%eax
	movl	%eax,0(%ecx)

	/* Do the system call */
	pushl	%ebx
	movl	16(%esp),%ebx
	movl	$SYS_ify(clone),%eax
	int	$0x80
	popl	%ebx

	test	%eax,%eax
	jl	SYSCALL_ERROR_LABEL
	jz	thread_start

L(pseudo_end):
	ret

thread_start:
	subl	%ebp,%ebp	/* terminate the stack frame */
	call	*%ebx
#ifdef PIC
	call	L(here)
L(here):
	popl	%ebx
	addl	$_GLOBAL_OFFSET_TABLE_+[.-L(here)], %ebx
#endif
	pushl	%eax
	call	JUMPTARGET (_exit)

PSEUDO_END (__clone)

weak_alias (__clone, clone)
