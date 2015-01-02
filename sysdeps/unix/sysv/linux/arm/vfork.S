/* Copyright (C) 1999-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Philip Blundell <philb@gnu.org>.

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
#define _ERRNO_H	1
#include <bits/errno.h>
#include <tcb-offsets.h>


/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

ENTRY (__vfork)
	/* Save the PID value.  */
	GET_TLS (r2)
	NEGOFF_ADJ_BASE2 (r2, r0, PID_OFFSET) /* Save the TLS addr in r2.  */
	ldr	r3, NEGOFF_OFF1 (r2, PID_OFFSET) /* Load the saved PID.  */
	rsbs	r0, r3, #0		/* Negate it, and test for zero.  */
	/* Use 0x80000000 if it was 0.  See raise.c for how this is used.  */
	it	eq
	moveq	r0, #0x80000000
	str	r0, NEGOFF_OFF1 (r2, PID_OFFSET) /* Store the temp PID.  */

	/* The DO_CALL macro saves r7 on the stack, to enable generation
	   of ARM unwind info.  Since the stack is initially shared between
	   parent and child of vfork, that saved value could be corrupted.
	   To avoid this problem, we save r7 into ip as well, and restore
	   from there.  */
	mov	ip, r7
	cfi_register (r7, ip)
	.fnstart
	push	{ r7 }
	cfi_adjust_cfa_offset (4)
	.save { r7 }
	ldr	r7, =SYS_ify (vfork)
	swi	0x0
	.fnend
	add	sp, sp, #4
	cfi_adjust_cfa_offset (-4)
	mov	r7, ip
	cfi_restore (r7)

	/* Restore the old PID value in the parent.  */
	cmp	r0, #0		/* If we are the parent... */
	it	ne
	strne	r3, NEGOFF_OFF1 (r2, PID_OFFSET) /* restore the saved PID.  */

	cmn	a1, #4096
	it	cc
	RETINSTR(cc, lr)

	b	PLTJMP(SYSCALL_ERROR)
PSEUDO_END (__vfork)
libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
strong_alias (__vfork, __libc_vfork)
