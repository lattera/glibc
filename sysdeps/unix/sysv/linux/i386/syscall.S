/* Copyright (C) 1995, 1996, 1998 Free Software Foundation, Inc.
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

/* Please consult the file sysdeps/unix/sysv/linux/i386/sysdep.h for
   more information about the value -4095 used below.*/

	.text
ENTRY (syscall)

	PUSHARGS_5		/* Save register contents.  */
	_DOARGS_5(36)		/* Load arguments.  */
	movl 16(%esp), %eax	/* Load syscall number into %eax.  */
	int $0x80		/* Do the system call.  */
	POPARGS_5		/* Restore register contents.  */
	cmpl $-4095, %eax	/* Check %eax for error.  */
	jae SYSCALL_ERROR_LABEL	/* Jump to error handler if error.  */
L(pseudo_end):
	ret			/* Return to caller.  */

PSEUDO_END (syscall)
