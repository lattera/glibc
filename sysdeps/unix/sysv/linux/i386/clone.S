/* Copyright (C) 1996 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sysdep.h>
#include <errnos.h>

/* int clone(int (*fn)(), void *child_stack, int flags, int nargs, ...) */

        .text
ENTRY(__clone)
	/* Sanity check arguments.  */
	movl	$-EINVAL,%eax
	movl	4(%esp),%ecx		/* no NULL function pointers */
	testl	%ecx,%ecx
	jz	syscall_error
	movl	8(%esp),%ecx		/* no NULL stack pointers */
	testl	%ecx,%ecx
	jz	syscall_error
	movl	16(%esp),%edx		/* no negative argument counts */
	testl	%edx,%edx
	js	syscall_error

	/* Allocate space on the new stack and copy args over */
	movl	%edx,%eax
	negl	%eax
	lea	-4(%ecx,%eax,4),%ecx
	jz	2f
1:	movl	16(%esp,%edx,4),%eax
	movl	%eax,0(%ecx,%edx,4)
	dec	%edx
	jnz	1b
2:
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
	jl	syscall_error
	jz	thread_start

	ret
	.size	__clone,.-__clone

	SYSCALL_ERROR_HANDLER

thread_start:
	subl	%ebp,%ebp	/* terminate the stack frame */
	call	*%ebx
	pushl	%eax
#ifdef PIC
	call	_exit@PLT
#else
	call	_exit
#endif

weak_alias(__clone, clone)
