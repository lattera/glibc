/* Copyright (C) 1999-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
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

	/* Pop the return PC value into ECX.  */
	popl	%ecx
	cfi_adjust_cfa_offset (-4)
	cfi_register (%eip, %ecx)

	/* Save the TCB-cached PID away in %edx, and then negate the TCB
           field.  But if it's zero, set it to 0x80000000 instead.  See
           raise.c for the logic that relies on this value.  */
	movl	%gs:PID, %edx
	movl	%edx, %eax
	negl	%eax
	jne	1f
	movl	$0x80000000, %eax
1:	movl	%eax, %gs:PID


	/* Stuff the syscall number in EAX and enter into the kernel.  */
	movl	$SYS_ify (vfork), %eax
	int	$0x80

	/* Jump to the return PC.  Don't jump directly since this
	   disturbs the branch target cache.  Instead push the return
	   address back on the stack.  */
	pushl	%ecx
	cfi_adjust_cfa_offset (4)

	/* Restore the original value of the TCB cache of the PID, if we're
	   the parent.  But in the child (syscall return value equals zero),
	   leave things as they are.  */
	testl	%eax, %eax
	je	1f
	movl	%edx, %gs:PID
1:

	cmpl	$-4095, %eax
	/* Branch forward if it failed.  */
	jae	SYSCALL_ERROR_LABEL

	ret

PSEUDO_END (__vfork)
libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
strong_alias (__vfork, __libc_vfork)
