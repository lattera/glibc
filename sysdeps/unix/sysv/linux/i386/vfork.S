/* Copyright (C) 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1999.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

ENTRY (__vfork)

	/* We have to use the clone() system call.  The flags to use are
		CLONE_VFORK | CLONE_VM
	   and we send SIGCHLD on termination (i.e., flags = 0x4111).  */

	popl	%edx		/* Pop return address.  */
	pushl	%ebx
	movl	%esp, %ecx
	movl	$0x4111, %ebx
	movl	$SYS_ify(clone), %eax
	int	$0x80
	popl	%ebx
	pushl	%edx
	cmpl	$-4095, %eax
	jae	SYSCALL_ERROR_LABEL
L(pseudo_end):
	ret

PSEUDO_END (__vfork)

weak_alias (__vfork, vfork)
