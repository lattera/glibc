/* Create new context.
   Copyright (C) 2001,02 Free Software Foundation, Inc.
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


ENTRY(__makecontext)
	movl	4(%esp), %eax

	/* Load the address of the function we are supposed to run.  */
	movl	8(%esp), %ecx

	/* Compute the address of the stack.  The information comes from
	   to us_stack element.  */
	movl	oSS_SP(%eax), %edx
	movl	%ecx, oEIP(%eax)
	addl	oSS_SIZE(%eax), %edx

	/* Put the next context on the new stack (from the uc_link
	   element).  */
	movl	oLINK(%eax), %ecx
	movl	%ecx, -4(%edx)

	/* Remember the number of parameters for the exit handler since
	   it has to remove them.  We store the number in the EBX register
	   which the function we will call must preserve.  */
	movl	12(%esp), %ecx
	movl	%ecx, oEBX(%eax)

	/* Make room on the new stack for the parameters.  */
	negl	%ecx
	leal	-8(%edx,%ecx,4), %edx
	negl	%ecx
	/* Store the future stack pointer.  */
	movl	%edx, oESP(%eax)

	/* Copy all the parameters.  */
	jecxz	2f
1:	movl	12(%esp,%ecx,4), %eax
	movl	%eax, (%edx,%ecx,4)
	decl	%ecx
	jnz	1b
2:

	/* If the function we call returns we must continue with the
	   context which is given in the uc_link element.  To do this
	   set the return address for the function the user provides
	   to a little bit of helper code which does the magic (see
	   below).  */
#ifdef PIC
	call	1f
1:	popl	%ecx
	addl	$L(exitcode)-1b, %ecx
	movl	%ecx, (%edx)
#else
	movl	$L(exitcode), (%edx)
#endif
	/* 'makecontext' returns no value.  */
	ret

	/* This is the helper code which gets called if a function which
	   is registered with 'makecontext' returns.  In this case we
	   have to install the context listed in the uc_link element of
	   the context 'makecontext' manipulated at the time of the
	   'makecontext' call.  If the pointer is NULL the process must
	   terminate.  */
L(exitcode):
	/* This removes the parameters passed to the function given to
	   'makecontext' from the stack.  EBX contains the number of
	   parameters (see above).  */
	leal	(%esp,%ebx,4), %esp

#ifdef	PIC
	call	1f
1:	popl	%ebx
	addl	$_GLOBAL_OFFSET_TABLE_+[.-1b], %ebx
#endif
	popl	%eax			/* This is the next context.  */
	testl	%eax, %eax
	je	2f			/* If it is zero exit.  */

	pushl	%eax
	call	JUMPTARGET(__setcontext)
	/* If this returns (which can happen if the syscall fails) we'll
	   exit the program with the return error value (-1).  */

2:	pushl	%eax
	call	HIDDEN_JUMPTARGET(exit)
	/* The 'exit' call should never return.  In case it does cause
	   the process to terminate.  */
	hlt
END(__makecontext)

weak_alias(__makecontext, makecontext)
