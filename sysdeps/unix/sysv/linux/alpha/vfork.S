/* Copyright (C) 2004-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <tcb-offsets.h>

ENTRY(__libc_vfork)
	PSEUDO_PROLOGUE

	/* Load the thread pointer value in A1 across the vfork.  */
	rduniq
	mov	v0, a1

	/* Save the TCB-cached PID away in A2, and then negate the TCB
           field.  But if it's zero, set it to 0x80000000 instead.  See
           raise.c for the logic that relies on this value.  */
	ldl	a2, PID_OFFSET(v0)
	ldah	t0, -0x8000
	negl	a2, t1
	cmovne	a2, t1, t0
	stl	t0, PID_OFFSET(v0);

	lda	v0, SYS_ify(vfork)
	call_pal PAL_callsys

	/* Restore the original value of the TCB cache of the PID, if we're
	   the parent.  But in the child (syscall return value equals zero),
	   leave things as they are.  */
	beq	v0, 1f
	stl	a2, PID_OFFSET(a1)
1:
	/* Normal error check and return.  */
	bne	a3, SYSCALL_ERROR_LABEL
	ret

PSEUDO_END (__libc_vfork)

#if IS_IN (libc)
weak_alias (__libc_vfork, vfork)
strong_alias (__libc_vfork, __vfork)
libc_hidden_def (__vfork)
#endif
