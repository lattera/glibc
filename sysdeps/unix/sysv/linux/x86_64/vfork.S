/* Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

#if SHSTK_ENABLED
/* The shadow stack prevents us from pushing the saved return PC onto
   the stack and returning normally.  Instead we pop the shadow stack
   and return directly.  This is the safest way to return and ensures
   any stack manipulations done by the vfork'd child doesn't cause the
   parent to terminate when CET is enabled.  */
# undef SYSCALL_ERROR_HANDLER
# define SYSCALL_ERROR_HANDLER			\
0:						\
  SYSCALL_SET_ERRNO;				\
  or $-1, %RAX_LP;				\
  jmp 1b;
# undef SYSCALL_ERROR_LABEL
# define SYSCALL_ERROR_LABEL 0f
#endif

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

	/* Stuff the syscall number in RAX and enter into the kernel.  */
	movl	$SYS_ify (vfork), %eax
	syscall

#if !SHSTK_ENABLED
	/* Push back the return PC.  */
	pushq	%rdi
	cfi_adjust_cfa_offset(8)
#endif

	cmpl	$-4095, %eax
	jae SYSCALL_ERROR_LABEL		/* Branch forward if it failed.  */

#if SHSTK_ENABLED
1:
	/* Check if shadow stack is in use.  */
	xorl	%esi, %esi
	rdsspq	%rsi
	testq	%rsi, %rsi
	/* Normal return if shadow stack isn't in use.  */
	je	L(no_shstk)

	/* Pop return address from shadow stack and jump back to caller
	   directly.  */
	movl	$1, %esi
	incsspq	%rsi
	jmp	*%rdi

L(no_shstk):
	/* Push back the return PC.  */
	pushq	%rdi
	cfi_adjust_cfa_offset(8)
#endif

	/* Normal return.  */
	ret

PSEUDO_END (__vfork)
libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
strong_alias (__vfork, __libc_vfork)
