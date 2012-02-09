/* Copyright (C) 2009, 2011 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@redhat.com>.
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
#include <init-arch.h>


/* Define multiple versions only for the definition in lib.  */
#ifndef NOT_IN_libc
	.text
ENTRY(rawmemchr)
	.type	rawmemchr, @gnu_indirect_function
	cmpl	$0, __cpu_features+KIND_OFFSET(%rip)
	jne	1f
	call	__init_cpu_features
1:	testl	$bit_Prefer_PMINUB_for_stringop, __cpu_features+FEATURE_OFFSET+index_Prefer_PMINUB_for_stringop(%rip)
	jnz	2f
	testl	$bit_SSE4_2, __cpu_features+CPUID_OFFSET+index_SSE4_2(%rip)
	jz	2f
	leaq	__rawmemchr_sse42(%rip), %rax
	ret
2:	leaq	__rawmemchr_sse2(%rip), %rax
	ret

END(rawmemchr)
strong_alias (rawmemchr, __rawmemchr)


	.section .text.sse4.2,"ax",@progbits
	.align	16
	.type	__rawmemchr_sse42, @function
__rawmemchr_sse42:
	cfi_startproc
	CALL_MCOUNT
	movd	%esi, %xmm1
	movq	%rdi, %rcx
	pxor	%xmm2, %xmm2
	andq	$~15, %rdi
	orl	$0xffffffff, %esi
	pshufb	%xmm2, %xmm1
	movdqa	(%rdi), %xmm0
	subq	%rdi, %rcx
	pcmpeqb	%xmm1, %xmm0
	shl	%cl, %esi
	pmovmskb %xmm0, %ecx
	movl	$16, %eax
	movl	$16, %edx
	andl	%esi, %ecx
	jnz	1f

2:	pcmpestri $0x08, 16(%rdi), %xmm1
	leaq	16(%rdi), %rdi
	jnc	2b

	leaq	(%rdi,%rcx), %rax
	ret

1:	bsfl	%ecx, %eax
	addq	%rdi, %rax
	ret
	cfi_endproc
	.size	__rawmemchr_sse42, .-__rawmemchr_sse42


# undef ENTRY
# define ENTRY(name) \
	.type __rawmemchr_sse2, @function; \
	.align 16; \
	__rawmemchr_sse2: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __rawmemchr_sse2, .-__rawmemchr_sse2
# undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal rawmemchr calls through a PLT.
   The speedup we get from using SSE4.2 instruction is likely eaten away
   by the indirect call in the PLT.  */
# define libc_hidden_builtin_def(name) \
	.globl __GI___rawmemchr; __GI___rawmemchr = __rawmemchr_sse2
#endif

#include "../rawmemchr.S"
