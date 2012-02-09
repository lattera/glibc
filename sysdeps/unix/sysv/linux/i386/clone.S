/* Copyright (C) 1996-2000,02,03,04,2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson (rth@tamu.edu)

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

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>
#include <asm-syntax.h>
#include <bp-sym.h>
#include <bp-asm.h>

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg,
	     pid_t *ptid, struct user_desc *tls, pid_t *ctid); */

#define PARMS	LINKAGE		/* no space for saved regs */
#define FUNC	PARMS
#define STACK	FUNC+4
#define FLAGS	STACK+PTR_SIZE
#define ARG	FLAGS+4
#define PTID	ARG+PTR_SIZE
#define TLS	PTID+PTR_SIZE
#define CTID	TLS+PTR_SIZE

#define __NR_clone 120
#define SYS_clone 120

#define CLONE_VM	0x00000100
#define CLONE_THREAD	0x00010000

        .text
ENTRY (BP_SYM (__clone))
	/* Sanity check arguments.  */
	movl	$-EINVAL,%eax
	movl	FUNC(%esp),%ecx		/* no NULL function pointers */
#ifdef PIC
	jecxz	SYSCALL_ERROR_LABEL
#else
	testl	%ecx,%ecx
	jz	SYSCALL_ERROR_LABEL
#endif
	movl	STACK(%esp),%ecx	/* no NULL stack pointers */
#ifdef PIC
	jecxz	SYSCALL_ERROR_LABEL
#else
	testl	%ecx,%ecx
	jz	SYSCALL_ERROR_LABEL
#endif

	/* Insert the argument onto the new stack.  Make sure the new
	   thread is started with an alignment of (mod 16).  */
	andl	$0xfffffff0, %ecx
	subl	$28,%ecx
	movl	ARG(%esp),%eax		/* no negative argument counts */
	movl	%eax,12(%ecx)

	/* Save the function pointer as the zeroth argument.
	   It will be popped off in the child in the ebx frobbing below.  */
	movl	FUNC(%esp),%eax
	movl	%eax,8(%ecx)
	/* Don't leak any information.  */
	movl	$0,4(%ecx)
#ifndef RESET_PID
	movl	$0,(%ecx)
#endif

	/* Do the system call */
	pushl	%ebx
	cfi_adjust_cfa_offset (4)
	pushl	%esi
	cfi_adjust_cfa_offset (4)
	pushl	%edi
	cfi_adjust_cfa_offset (4)

	movl	TLS+12(%esp),%esi
	cfi_rel_offset (esi, 4)
	movl	PTID+12(%esp),%edx
	movl	FLAGS+12(%esp),%ebx
	cfi_rel_offset (ebx, 8)
	movl	CTID+12(%esp),%edi
	cfi_rel_offset (edi, 0)
	movl	$SYS_ify(clone),%eax

#ifdef RESET_PID
	/* Remember the flag value.  */
	movl	%ebx, (%ecx)
#endif

	/* End FDE now, because in the child the unwind info will be
	   wrong.  */
	cfi_endproc

	int	$0x80
	popl	%edi
	popl	%esi
	popl	%ebx

	test	%eax,%eax
	jl	SYSCALL_ERROR_LABEL
	jz	L(thread_start)

L(pseudo_end):
	ret

L(thread_start):
	cfi_startproc;
	/* Clearing frame pointer is insufficient, use CFI.  */
	cfi_undefined (eip);
	/* Note: %esi is zero.  */
	movl	%esi,%ebp	/* terminate the stack frame */
#ifdef RESET_PID
	testl	$CLONE_THREAD, %edi
	je	L(newpid)
L(haspid):
#endif
	call	*%ebx
#ifdef PIC
	call	L(here)
L(here):
	popl	%ebx
	addl	$_GLOBAL_OFFSET_TABLE_+[.-L(here)], %ebx
#endif
	movl	%eax, %ebx
	movl	$SYS_ify(exit), %eax
	ENTER_KERNEL

#ifdef RESET_PID
	.subsection 2
L(newpid):
	testl	$CLONE_VM, %edi
	movl	$-1, %eax
	jne	L(nomoregetpid)
	movl	$SYS_ify(getpid), %eax
	ENTER_KERNEL
L(nomoregetpid):
	movl	%eax, %gs:PID
	movl	%eax, %gs:TID
	jmp	L(haspid)
	.previous
#endif
	cfi_endproc;

	cfi_startproc
PSEUDO_END (BP_SYM (__clone))

weak_alias (BP_SYM (__clone), BP_SYM (clone))
