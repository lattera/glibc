/* Copyright (C) 1991, 1992, 1995, 1997 Free Software Foundation, Inc.
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

#ifndef	SYS_vfork
#define	SYS_vfork	66
#endif

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */
.globl ___vfork
___vfork:
	.word 0
	/* Save our return address in R2, and return to code below.  */
	movl 16(fp), r2
	movab unwind, 16(fp)
	ret
unwind:
	/* Do the system call.  */
	chmk $SYS_vfork
	bcs error

	tstl r1
	beq parent

	/* We are the child.  Return zero.  */
	clrl r0

	/* Return to the saved address.  */
parent:	jmp (r2)

.globl	_errno
error:
	movl r0, _errno
	mnegl $1, r0
	jmp (r2)

weak_alias (__vfork, vfork)
