/* Copyright (C) 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

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

#include <sysdep.h>
#include <errnos.h>

	.text
	SYSCALL_ERROR_HANDLER	/* Error handler for PIC.  */
ENTRY (__xmknod)
	popl %ecx		/* Pop return address into %eax.  */
	popl %edx		/* Pop version number into %edx.  */
	pushl %ecx		/* Push back the return address.  */

	cmpl $1, %edx		/* Version 1 xstat call?  */
	je .L1			/* Yes, do syscall.  */

	movl $-EINVAL, %eax	/* Load error code into %eax.  */
	jmp .L2

.L1:	DO_CALL (3, mknod)	/* Do the syscall.   */

	movl (%esp), %ecx
	testl %eax, %eax	/* Check for error.  */
	jl .L2

	jmp *%ecx		/* Return success.  */

.L2:	pushl %ecx
	jmp syscall_error
				/* Yes, then branch to error handling.  */

/* For compatibility with Linux libc.  */
weak_alias (__xmknod, _xmknod)
