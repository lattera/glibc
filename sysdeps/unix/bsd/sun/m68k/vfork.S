/* Copyright (C) 1991, 92, 93, 94, 95, 97, 2002 Free Software Foundation, Inc.
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
	/* Pop the return PC value into A0.  */
	movel sp@+, a0

	/* Push the syscall number and trap into the kernel.  */
	movel #SYS_vfork, sp@-
	trap #0
	bcs error		/* Branch forward if it failed.  */

	/* It succeeded.  See which fork we're in.  D1 is now 0 for the
	   parent and 1 for the child.  Decrement it to make it -1 (all
	   bits set) for the parent, and 0 (no bits set) for the child.
	   Then AND it with D0, so the parent gets D0&-1==R0, and the child
	   gets D0&0==0.  */
	subql #1, d1
	andl d1, d0

	/* Jump to the return PC.  */
	jmp a0@

error:
	movel d0, _errno
	moveq #-1, d0
	jmp a0@
libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
