/* Copyright (C) 2001, 2002, 2003 Free Software Foundation, Inc.
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

#include <sys/asm.h>

/* Please consult the file sysdeps/unix/sysv/linux/x86-64/sysdep.h for
   more information about the value -4095 used below.  */

/* Usage: long syscall (syscall_number, arg1, arg2, arg3, arg4, arg5)
   We need to do some arg shifting, the syscall_number will be in
   rax.  */


	.text
ENTRY (syscall)
	move v0, a0		/* Syscall number -> v0 */
	move a0, a1		/* shift arg1 - arg7.  */
	move a1, a2
	move a2, a3
	move a3, a4
	move a4, a5
	move a5, a6
	move a6, a7

	syscall			/* Do the system call.  */
	bne a3, zero, L(error)

	ret

L(error):
	.cpsetup t9, a0, syscall
	PTR_LA t9,__syscall_error
	.cprestore
	jr t9

PSEUDO_END (syscall)
