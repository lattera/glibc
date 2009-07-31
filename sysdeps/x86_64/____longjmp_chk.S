/* Copyright (C) 2001,2004,2005,2006,2009 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <jmpbuf-offsets.h>
#include <asm-syntax.h>

	.section .rodata.str1.1,"aMS",@progbits,1
	.type	longjmp_msg,@object
longjmp_msg:
	.string "longjmp causes uninitialized stack frame"
	.size	longjmp_msg, .-longjmp_msg


//#define __longjmp ____longjmp_chk

#ifdef PIC
# define CALL_FAIL	leaq	longjmp_msg(%rip), %rdi;		      \
			call	__GI___fortify_fail
#else
# define CALL_FAIL	movq	$longjmp_msg, %rdi;			      \
			call	__fortify_fail
#endif

#define CHECK_RSP(reg) \
	cmpq	reg, %rsp;						      \
	jbe	.Lok;							      \
	CALL_FAIL;							      \
.Lok:

/* Jump to the position specified by ENV, causing the
   setjmp call there to return VAL, or 1 if VAL is 0.
   void __longjmp (__jmp_buf env, int val).  */
	.text
ENTRY(____longjmp_chk)
	/* Restore registers.  */
	movq	(JB_RSP*8)(%rdi),%r8
	movq	(JB_RBP*8)(%rdi),%r9
	movq	(JB_PC*8)(%rdi),%rdx
#ifdef PTR_DEMANGLE
	PTR_DEMANGLE (%r8)
	PTR_DEMANGLE (%r9)
	PTR_DEMANGLE (%rdx)
#endif

	/* Save function parameters.  */
	movq	%rdi, %r10
	movl	%esi, %ecx

	xorl	%eax, %eax
	cmpq	%r8, %rsp
	jbe	.Lok

	subq	$24, %rsp
	cfi_adjust_cfa_offset(24)
	xorl	%edi, %edi
	movq	%rsp, %rsi
	movl	$__NR_sigaltstack, %eax
	syscall
	testl	%eax, %eax
	movl	$0, %eax
	jne	.Lok
	movl	8(%rsp), %eax
	andl	$1, %eax

.Lok:
	/* We add unwind information for the target here.  */
	cfi_def_cfa(%r10, 0)
	cfi_register(%rsp,%r8)
	cfi_register(%rbp,%r9)
	cfi_register(%rip,%rdx)
	cfi_offset(%rbx,JB_RBX*8)
	cfi_offset(%r12,JB_R12*8)
	cfi_offset(%r13,JB_R13*8)
	cfi_offset(%r14,JB_R14*8)
	cfi_offset(%r15,JB_R15*8)

	xchgq	%r8, %rsp
	cfi_restore(%rsp)
	xchgq	%r9, %rbp
	cfi_restore(%rbp)

	movq	(JB_RBX*8)(%r10),%rbx
	movq	(JB_R12*8)(%r10),%r12
	movq	(JB_R13*8)(%r10),%r13
	movq	(JB_R14*8)(%r10),%r14
	movq	(JB_R15*8)(%r10),%r15

	cmpq	%rsp, %r8
	jnbe	.Lcheck

	/* Set return value for setjmp.  */
.Lout:	movl	%ecx, %eax
	jmpq	*%rdx

.Lcheck:
	testl	%eax, %eax
	je	.Lfail

	subq	$24, %rsp
	cfi_adjust_cfa_offset(24)
	xorl	%edi, %edi
	movq	%rsp, %rsi
	movl	$__NR_sigaltstack, %eax
	syscall
	addq	$24, %rsp
	cfi_adjust_cfa_offset(-24)
	testl	$1, 8(%rsp)
	je	.Lout

.Lfail:	xchgq	%r8, %rsp
	/* We want the stack trace to show that of the caller.  */
	cfi_def_cfa(%rsp, 32)
	cfi_restore(%rsp)
	cfi_register(%rbp, %r9)
	cfi_restore(%rip)
	cfi_restore(%rbx)
	cfi_restore(%r12)
	cfi_restore(%r13)
	cfi_restore(%r14)
	cfi_restore(%r15)

	xchgq	%r9, %rbp
	cfi_restore(%rbp)

	CALL_FAIL
	hlt
END (BP_SYM (____longjmp_chk))
