/* Copyright (C) 1999, 2002, 2004 Free Software Foundation, Inc.
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
#include <kernel-features.h>

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

ENTRY (__vfork)

#ifdef __NR_vfork

	/* Pop the return PC value into ECX.  */
	popl	%ecx

#ifdef SAVE_PID
	SAVE_PID
#endif

	/* Stuff the syscall number in EAX and enter into the kernel.  */
	movl	$SYS_ify (vfork), %eax
	int	$0x80

	/* Jump to the return PC.  Don't jump directly since this
	   disturbs the branch target cache.  Instead push the return
	   address back on the stack.  */
	pushl	%ecx

#ifdef RESTORE_PID
	RESTORE_PID
#endif

	cmpl	$-4095, %eax
	/* Branch forward if it failed.  */
# ifdef __ASSUME_VFORK_SYSCALL
	jae	SYSCALL_ERROR_LABEL
.Lpseudo_end:
# else
	jae	.Lerror
# endif

	ret

# ifndef __ASSUME_VFORK_SYSCALL
.Lerror:
	/* Check if vfork syscall is known at all.  */
	cmpl	$-ENOSYS, %eax
	jne	SYSCALL_ERROR_LABEL
# endif
#endif

#ifndef __ASSUME_VFORK_SYSCALL
	/* If we don't have vfork, fork is close enough.  */

	movl	$SYS_ify (fork), %eax
	int	$0x80
	cmpl	$-4095, %eax
	jae	SYSCALL_ERROR_LABEL
.Lpseudo_end:
	ret
#endif
PSEUDO_END (__vfork)
libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
