/* Copyright (C) 2001,2004,2005,2006,2009,2010,2011 Free Software Foundation, Inc.
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
# define CALL_FAIL	subq	$8, %rsp;				      \
			cfi_remember_state;				      \
			cfi_def_cfa_offset(16);				      \
			leaq	longjmp_msg(%rip), %rdi;		      \
			call	HIDDEN_JUMPTARGET(__fortify_fail);	      \
			nop;						      \
			cfi_restore_state
#else
# define CALL_FAIL	subq	$8, %rsp;				      \
			cfi_remember_state;				      \
			cfi_def_cfa_offset(16);				      \
			movq	$longjmp_msg, %rdi;			      \
			call	HIDDEN_JUMPTARGET(__fortify_fail);	      \
			nop;						      \
			cfi_restore_state
#endif

/* Jump to the position specified by ENV, causing the
   setjmp call there to return VAL, or 1 if VAL is 0.
   void __longjmp (__jmp_buf env, int val).  */
	.text
ENTRY(____longjmp_chk)
	/* Restore registers.  */
	movq	(JB_RSP*8)(%rdi), %r8
	movq	(JB_RBP*8)(%rdi), %r9
	movq	(JB_PC*8)(%rdi), %rdx
#ifdef PTR_DEMANGLE
	PTR_DEMANGLE (%r8)
	PTR_DEMANGLE (%r9)
	PTR_DEMANGLE (%rdx)
#endif

	cmpq	%r8, %rsp
	jbe	.Lok

	/* Save function parameters.  */
	movq	%rdi, %r10
	cfi_register (%rdi, %r10)
	movl	%esi, %ebx
	cfi_register (%rsi, %rbx)

	xorl	%edi, %edi
	leaq	-24(%rsp), %rsi
	movl	$__NR_sigaltstack, %eax
	syscall
	/* Without working sigaltstack we cannot perform the test.  */
	testl	%eax, %eax
	jne	.Lok2
	testl	$1, -16(%rsp)
	jz	.Lfail

	movq	-24(%rsp), %rax
	addq	-8(%rsp), %rax
	subq	%r8, %rax
	cmpq	-8(%rsp), %rax
	jae	.Lok2

.Lfail:	CALL_FAIL

.Lok2:	movq	%r10, %rdi
	cfi_restore (%rdi)
	movl	%ebx, %esi
	cfi_restore (%rsi)

.Lok:	/* We add unwind information for the target here.  */
	cfi_def_cfa(%rdi, 0)
	cfi_register(%rsp,%r8)
	cfi_register(%rbp,%r9)
	cfi_register(%rip,%rdx)
	cfi_offset(%rbx,JB_RBX*8)
	cfi_offset(%r12,JB_R12*8)
	cfi_offset(%r13,JB_R13*8)
	cfi_offset(%r14,JB_R14*8)
	cfi_offset(%r15,JB_R15*8)
	movq	(JB_RBX*8)(%rdi), %rbx
	movq	(JB_R12*8)(%rdi), %r12
	movq	(JB_R13*8)(%rdi), %r13
	movq	(JB_R14*8)(%rdi), %r14
	movq	(JB_R15*8)(%rdi), %r15
	/* Set return value for setjmp.  */
	movl	%esi, %eax
	movq	%r8,%rsp
	movq	%r9,%rbp
	jmpq	*%rdx
END (____longjmp_chk)
