/* Copyright (C) 2004-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2004.

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
#define _ERRNO_H	1
#include <bits/errno.h>
#include <bits/wordsize.h>
#include <tcb-offsets.h>

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

ENTRY (__libc_vfork)
	ear	%r4,%a0
	sllg	%r4,%r4,32
	ear	%r4,%a1
	icm	%r3,15,PID(%r4)
	llilh	%r1,32768
	je	1f
	lcr	%r1,%r3
1:	st	%r1,PID(%r4)

	/* Do vfork system call.  */
	svc	SYS_ify (vfork)

	ltgr	%r2,%r2
	je	1f
	st	%r3,PID(%r4)
1:
	/* Check for error.  */
	lghi	%r4,-4095
	clgr	%r2,%r4
	jgnl	SYSCALL_ERROR_LABEL

	/* Normal return.  */
	br	%r14
PSEUDO_END (__libc_vfork)

#if IS_IN (libc)
weak_alias (__libc_vfork, vfork)
strong_alias (__libc_vfork, __vfork)
libc_hidden_def (__vfork)
#endif
