/* Copyright (C) 1993, 1994, 1997 Free Software Foundation, Inc.
   Contributed by Scott Bartram.
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

/*
    before lcall, stack contents should be:

	4(%esp) -> name
	8(%esp) -> unspecified
	12(%esp) -> 0
 */

ENTRY (uname)
	pushl $0x0		/* Push the discriminator flag.  */
	pushl $0x0		/* Push dummy placeholder.  */
	pushl 12(%esp,1)	/* Push NAME (ptr to struct utsname)  */
	subl $0x4, %esp		/* Adjust stack pointer.  */
	DO_CALL (utssys, 3)
	jb error		/* Test for error.  */
	addl $0x10, %esp	/* Adjust the stack pointer.  */
	xorl %eax, %eax		/* Clear return value.  */
	ret
error:	addl $0x10, %esp	/* Adjust the stack pointer.  */
	jmp syscall_error
