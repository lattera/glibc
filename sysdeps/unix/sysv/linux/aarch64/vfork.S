/* Copyright (C) 1999-2015 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

ENTRY (__vfork)

	/* Save the TCB-cached PID away in w3, and then negate the TCB
           field.  But if it's zero, set it to 0x80000000 instead.  See
           raise.c for the logic that relies on this value.  */
	mrs	x2, tpidr_el0
	sub	x2, x2, #PTHREAD_SIZEOF
	ldr	w3, [x2, #PTHREAD_PID_OFFSET]
	mov	w1, #0x80000000
	negs	w0, w3
	csel	w0, w1, w0, eq
	str	w0, [x2, #PTHREAD_PID_OFFSET]

	mov	x0, #0x4111	/* CLONE_VM | CLONE_VFORK | SIGCHLD */
	mov	x1, sp
	DO_CALL (clone, 2)

	/* Restore the original value of the TCB cache of the PID, if we're
	   the parent.  But in the child (syscall return value equals zero),
	   leave things as they are.  */
	cbz	x0, 1f
	str	w3, [x2, #PTHREAD_PID_OFFSET]
1:
	cmn	x0, #4095
	b.cs    .Lsyscall_error
	RET

PSEUDO_END (__vfork)
libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
strong_alias (__vfork, __libc_vfork)
