/* vfork for Nios II Linux.
   Copyright (C) 2005-2015 Free Software Foundation, Inc.
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
#include <tcb-offsets.h>

ENTRY(__vfork)

	ldw	r6, PID_OFFSET(r23)
	sub	r7, zero, r6
	bne	r7, zero, 2f
	movhi	r7, %hi(0x80000000)
2:
	stw	r7, PID_OFFSET(r23)

	movi	r4, 0x4111 /* (CLONE_VM | CLONE_VFORK | SIGCHLD) */
	mov	r5, zero
	DO_CALL (clone, 2)

	beq	r2, zero, 1f
	stw	r6, PID_OFFSET(r23)
1:
	bne	r7, zero, SYSCALL_ERROR_LABEL
	ret

PSEUDO_END (__vfork)
libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
