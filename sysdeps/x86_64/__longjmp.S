/* Copyright (C) 2001-2016 Free Software Foundation, Inc.
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
#include <jmpbuf-offsets.h>
#include <asm-syntax.h>
#include <stap-probe.h>

/* Jump to the position specified by ENV, causing the
   setjmp call there to return VAL, or 1 if VAL is 0.
   void __longjmp (__jmp_buf env, int val).  */
	.text
ENTRY(__longjmp)
	/* Restore registers.  */
	mov (JB_RSP*8)(%rdi),%R8_LP
	mov (JB_RBP*8)(%rdi),%R9_LP
	mov (JB_PC*8)(%rdi),%RDX_LP
#ifdef PTR_DEMANGLE
	PTR_DEMANGLE (%R8_LP)
	PTR_DEMANGLE (%R9_LP)
	PTR_DEMANGLE (%RDX_LP)
# ifdef __ILP32__
	/* We ignored the high bits of the %rbp value because only the low
	   bits are mangled.  But we cannot presume that %rbp is being used
	   as a pointer and truncate it, so recover the high bits.  */
	movl (JB_RBP*8 + 4)(%rdi), %eax
	shlq $32, %rax
	orq %rax, %r9
# endif
#endif
	LIBC_PROBE (longjmp, 3, LP_SIZE@%RDI_LP, -4@%esi, LP_SIZE@%RDX_LP)
	/* We add unwind information for the target here.  */
	cfi_def_cfa(%rdi, 0)
	cfi_register(%rsp,%r8)
	cfi_register(%rbp,%r9)
	cfi_register(%rip,%rdx)
	cfi_offset(%rbx,JB_RBX*8)
	cfi_offset(%r12,JB_R12*8)
	cfi_offset(%r13,JB_R13*8)
	cfi_offset(%r14,JB_R14*8)
	cfi_offset(%r15,JB_R15*8)
	movq (JB_RBX*8)(%rdi),%rbx
	movq (JB_R12*8)(%rdi),%r12
	movq (JB_R13*8)(%rdi),%r13
	movq (JB_R14*8)(%rdi),%r14
	movq (JB_R15*8)(%rdi),%r15
	/* Set return value for setjmp.  */
	mov %esi, %eax
	mov %R8_LP,%RSP_LP
	movq %r9,%rbp
	LIBC_PROBE (longjmp_target, 3,
		    LP_SIZE@%RDI_LP, -4@%eax, LP_SIZE@%RDX_LP)
	jmpq *%rdx
END (__longjmp)
