/* Copyright (C) 1995, 1996 Free Software Foundation, Inc.
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

ENTRY (__mmap)

	/* Save registers.  */
	movl %ebx, %edx

	movl $SYS_ify(mmap), %eax	/* System call number in %eax.  */

	lea 4(%esp), %ebx		/* Address of args is 1st arg.  */

        /* Do the system call trap.  */
	int $0x80

	/* Restore registers.  */
	movl %edx, %ebx

	/* If 0 > %eax > -4096 there was an error.  */
	cmpl $-4096, %eax
	ja syscall_error

	/* Successful; return the syscall's value.  */
	ret

PSEUDO_END (__mmap)

weak_alias (__mmap, mmap)
