/* Copyright (C) 2001-2015 Free Software Foundation, Inc.
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

	/* Pop the return PC value into RDI.  We need a register that
	   is preserved by the syscall and that we're allowed to destroy. */
	popq	%rdi
	cfi_adjust_cfa_offset(-8)
	cfi_register(%rip, %rdi)

	/* Save the TCB-cached PID away in %esi, and then negate the TCB
           field.  But if it's zero, set it to 0x80000000 instead.  See
           raise.c for the logic that relies on this value.  */
	movl	%fs:PID, %esi
	movl	$0x80000000, %ecx
	movl	%esi, %edx
	negl	%edx
	cmove	%ecx, %edx
	movl	%edx, %fs:PID

	/* Stuff the syscall number in RAX and enter into the kernel.  */
	movl	$SYS_ify (vfork), %eax
	syscall

	/* Push back the return PC.  */
	pushq	%rdi
	cfi_adjust_cfa_offset(8)

	/* Restore the original value of the TCB cache of the PID, if we're
	   the parent.  But in the child (syscall return value equals zero),
	   leave things as they are.  */
	testq	%rax, %rax
	je	1f
	movl	%esi, %fs:PID
1:

	cmpl	$-4095, %eax
	jae SYSCALL_ERROR_LABEL		/* Branch forward if it failed.  */

	/* Normal return.  */
	ret

PSEUDO_END (__vfork)
libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
strong_alias (__vfork, __libc_vfork)
