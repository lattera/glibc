/* Copyright (C) 1995-1998,2002,2003,2005,2008 Free Software Foundation, Inc.
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

#include <sysdep-cancel.h>
#include <socketcall.h>
#include <tls.h>

#define _NSIG 64

#define P(a, b) P2(a, b)
#define P2(a, b) a##b

	.text
/* The socket-oriented system calls are handled unusally in Linux.
   They are all gated through the single `socketcall' system call number.
   `socketcall' takes two arguments: the first is the subcode, specifying
   which socket function is being called; and the second is a pointer to
   the arguments to the specific function.  */

ENTRY(do_paccept)
#ifdef SOCKOP_paccept
	subl $6*4, %esp
	cfi_adjust_cfa_offset(6*4)

	movl (%eax), %ecx
	movl %ecx, (%esp)
	movl 4(%eax), %ecx
	movl %ecx, 4(%esp)
	movl 8(%eax), %ecx
	movl %ecx, 8(%esp)
	movl 12(%eax), %ecx
	movl %ecx, 12(%esp)
	movl $(_NSIG / 8), 16(%esp)
	movl 16(%eax), %ecx
	movl %ecx, 20(%esp)

	movl $SYS_ify(socketcall), %eax	/* System call number in %eax.  */

	movl $SOCKOP_paccept, %ebx	/* Subcode is first arg to syscall.  */
	movl %esp, %ecx			/* Address of args is 2nd arg.  */

        /* Do the system call trap.  */
	ENTER_KERNEL

	addl $6*4, %esp
	cfi_adjust_cfa_offset(-6*4)

	/* %eax is < 0 if there was an error.  */
	cmpl $-125, %eax
	jae SYSCALL_ERROR_LABEL
#else
	movl $-ENOSYS, %eax
	jmp SYSCALL_ERROR_LABEL

	.section .gnu.glibc-stub.paccept
	.previous
	.section .gnu.warning.paccept
	.string "warning: paccept is not implemented and will always fail"
	.previous
#endif
L(pseudo_end):
	ret
PSEUDO_END(do_paccept)


	.globl paccept
ENTRY (paccept)
#ifdef CENABLE
	SINGLE_THREAD_P
	jne 1f
#endif

	/* Save registers.  */
	movl %ebx, %edx
	cfi_register (3, 2)

	lea 4(%esp), %eax
	call do_paccept

	/* Restore registers.  */
	movl %edx, %ebx
	cfi_restore (3)

	ret

#ifdef CENABLE
	/* We need one more register.  */
1:	pushl %esi
	cfi_adjust_cfa_offset(4)

	/* Enable asynchronous cancellation.  */
	CENABLE
	movl %eax, %esi
	cfi_offset(6, -8)		/* %esi */

	/* Save registers.  */
	movl %ebx, %edx
	cfi_register (3, 2)

	lea 8(%esp), %eax
	call do_paccept

	/* Restore registers.  */
	movl %edx, %ebx
	cfi_restore (3)

	/* Restore the cancellation.  */
	xchgl %esi, %eax
	CDISABLE

	/* Restore registers.  */
	movl %esi, %eax
	popl %esi
	cfi_restore (6)
	cfi_adjust_cfa_offset(-4)

	/* Successful; return the syscall's value.  */
	ret
#endif
PSEUDO_END (paccept)
