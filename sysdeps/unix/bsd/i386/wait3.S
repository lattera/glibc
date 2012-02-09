/* Copyright (C) 1991, 1992, 1993, 1995, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

/* <sysdeps/unix/bsd/sequent/i386/sysdep.h> defines this to put the first
   two arguments into registers.  Since the arguments to wait3 are
   transferred magically through the same registers, we want to disable this.
   This allows us to avoid rewriting this file for that system.  */

#undef	ARGS_2
#define ARGS_2	/* Special-case no-op.  */

.text
.globl syscall_error
.align 4
ENTRY (__wait3)
	mov 8(%esp), %ecx	/* Flags.  */
	mov 12(%esp), %edx	/* rusage pointer.  */
	pushl $0xdf; popf	/* Set all the condition codes.  */
	DO_CALL (wait, 2)	/* Do the system call.  */
	je syscall_error	/* Check for error.  */
	mov 4(%esp), scratch	/* Status pointer.  */
	orl scratch, scratch	/* Is it nil?  */
	je done			/* Yup; return.  */
	mov r1, 0(scratch)	/* Non-nil; store the status in it.  */
done:	ret

weak_alias (__wait3, wait3)
