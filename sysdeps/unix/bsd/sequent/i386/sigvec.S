/* Copyright (C) 1993, 1995, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

/* The Dynix `sigvec' system call takes an extra argument,
   which is the address of the trampoline function.  */

.text
.align 4
trampoline:
	cld			/* Clear direction flag.  */
	call %eax		/* Call the handler, address in %eax.  */
	addl $8, %esp		/* Pop signum & code off the stack.  */
	/* __sigreturn will restore the context, and never return here.  */
	call C_SYMBOL_NAME (__sigreturn)

.globl syscall_error
ENTRY (__sigvec)
	pushl $trampoline	/* Push fourth arg: trampoline address.  */
	pushl 16(%esp)		/* Push third arg: our third arg.  */
	pushl 16(%esp)		/* Push second arg: our second arg.  */
	pushl 16(%esp)		/* Push first arg: our first arg.  */
	mov %esp, %ecx 		/* Point the syscall at the arguments.  */
	addl $16, %esp		/* Pop those four args.  */
	DO_CALL (sigvec, 4)	/* Do the system call.  */
	jb syscall_error	/* Check for error.  */
	ret

weak_alias (__sigvec, sigvec)
