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

	.text
	SYSCALL_ERROR_HANDLER

ENTRY (__mmap)

	/* Save registers.  */
	movl %ebx, %edx

	movl $SYS_ify(mmap), %eax	/* System call number in %eax.  */

	lea 4(%esp), %ebx		/* Address of args is 1st arg.  */

        /* Do the system call trap.  */
	int $0x80

	/* Restore registers.  */
	movl %edx, %ebx

	/* %eax is < 0 if there was an error.  */
	testl %eax, %eax
	jl JUMPTARGET(syscall_error)

	/* Successful; return the syscall's value.  */
	ret

weak_alias (__mmap, mmap)
