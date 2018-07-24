/* Copyright (C) 1999-2018 Free Software Foundation, Inc.
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

#if SHSTK_ENABLED
/* The shadow stack prevents us from pushing the saved return PC onto
   the stack and returning normally.  Instead we pop the shadow stack
   and return directly.  This is the safest way to return and ensures
   any stack manipulations done by the vfork'd child doesn't cause the
   parent to terminate when CET is enabled.  */
# undef SYSCALL_ERROR_HANDLER
# ifdef PIC
#  define SYSCALL_ERROR_HANDLER				\
0:							\
  calll .L1;						\
.L1:							\
  popl %edx;						\
.L2:							\
  addl $_GLOBAL_OFFSET_TABLE_ + (.L2 - .L1), %edx;	\
  movl __libc_errno@gotntpoff(%edx), %edx;		\
  negl %eax;						\
  movl %eax, %gs:(%edx);				\
  orl $-1, %eax;					\
  jmp 1b;
# else
#  define SYSCALL_ERROR_HANDLER				\
0:							\
  movl __libc_errno@indntpoff, %edx;			\
  negl %eax;						\
  movl %eax, %gs:(%edx);				\
  orl $-1, %eax;					\
  jmp 1b;
# endif
# undef SYSCALL_ERROR_LABEL
# define SYSCALL_ERROR_LABEL 0f
#endif

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

ENTRY (__vfork)

	/* Pop the return PC value into ECX.  */
	popl	%ecx
	cfi_adjust_cfa_offset (-4)
	cfi_register (%eip, %ecx)

	/* Stuff the syscall number in EAX and enter into the kernel.  */
	movl	$SYS_ify (vfork), %eax
	int	$0x80

#if !SHSTK_ENABLED
	/* Jump to the return PC.  Don't jump directly since this
	   disturbs the branch target cache.  Instead push the return
	   address back on the stack.  */
	pushl	%ecx
	cfi_adjust_cfa_offset (4)
#endif

	cmpl	$-4095, %eax
	/* Branch forward if it failed.  */
	jae	SYSCALL_ERROR_LABEL

#if SHSTK_ENABLED
1:
	/* Check if shadow stack is in use.  */
	xorl	%edx, %edx
	rdsspd	%edx
	testl	%edx, %edx
	/* Normal return if shadow stack isn't in use.  */
	je	L(no_shstk)

	/* Pop return address from shadow stack and jump back to caller
	   directly.  */
	movl	$1, %edx
	incsspd	%edx
	jmp	*%ecx

L(no_shstk):
	/* Jump to the return PC.  Don't jump directly since this
	   disturbs the branch target cache.  Instead push the return
	   address back on the stack.  */
	pushl	%ecx
	cfi_adjust_cfa_offset (4)
#endif

	ret

PSEUDO_END (__vfork)
libc_hidden_def (__vfork)

weak_alias (__vfork, vfork)
strong_alias (__vfork, __libc_vfork)
