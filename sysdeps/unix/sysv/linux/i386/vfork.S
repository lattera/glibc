/* Copyright (C) 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Schwab <schwab@gnu.org>.

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
#define _ERRNO_H	1
#include <bits/errno.h>

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

ENTRY (__vfork)

#ifdef __NR_vfork

	/* Pop the return PC value into ECX.  */
	popl	%ecx

	/* Stuff the syscall number in EAX and enter into the kernel.  */
	movl	$SYS_ify (vfork), %eax
	int	$0x80
	cmpl	$-4095, %eax
	jae	.Lerror		/* Branch forward if it failed.  */

	/* Jump to the return PC.  */
	jmp	*%ecx

.Lerror:
	/* Push back the return PC.  */
	pushl	%ecx

	/* Check if vfork syscall is known at all.  */
	cmpl	$-ENOSYS, %eax
	jne	SYSCALL_ERROR_LABEL

#endif

	/* If we don't have vfork, fork is close enough.  */

	movl	$SYS_ify (fork), %eax
	int	$0x80
	cmpl	$-4095, %eax
	jae	SYSCALL_ERROR_LABEL
.Lpseudo_end:
	ret

PSEUDO_END (__vfork)

weak_alias (__vfork, vfork)
