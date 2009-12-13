/* strlen(str) -- determine the length of the string STR.
   Copyright (C) 2009 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include <init-arch.h>


/* Define multiple versions only for the definition in libc and for
   the DSO.  In static binaries we need strlen before the initialization
   happened.  */
#if defined SHARED && !defined NOT_IN_libc
	.text
ENTRY(strlen)
	.type	strlen, @gnu_indirect_function
	cmpl	$0, __cpu_features+KIND_OFFSET(%rip)
	jne	1f
	call	__init_cpu_features
1:	leaq	__strlen_sse2(%rip), %rax
	testl	$bit_SSE4_2, __cpu_features+CPUID_OFFSET+index_SSE4_2(%rip)
	jz	2f
	leaq	__strlen_sse42(%rip), %rax
2:	ret
END(strlen)


	.section .text.sse4.2,"ax",@progbits
	.align 	16
	.type	__strlen_sse42, @function
__strlen_sse42:
	cfi_startproc
	CALL_MCOUNT
	pxor	%xmm2, %xmm2
	movq	%rdi, %rcx
	movq	%rdi, %r8
	andq	$~15, %rdi
	movdqa	%xmm2, %xmm1
	pcmpeqb	(%rdi), %xmm2
	orl	$0xffffffff, %esi
	subq	%rdi, %rcx
	shll	%cl, %esi
	pmovmskb %xmm2, %edx
	andl	%esi, %edx
	jnz	1f

2:	pcmpistri $0x08, 16(%rdi), %xmm1
	leaq	16(%rdi), %rdi
	jnz	2b

	leaq	(%rdi,%rcx), %rax
	subq	%r8, %rax
	ret

1:	subq	%r8, %rdi
	bsfl	%edx, %eax
	addq	%rdi, %rax
	ret
	cfi_endproc
	.size	__strlen_sse42, .-__strlen_sse42


# undef ENTRY
# define ENTRY(name) \
	.type __strlen_sse2, @function; \
	.align 16; \
	__strlen_sse2: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size __strlen_sse2, .-__strlen_sse2
# undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal strlen calls through a PLT.
   The speedup we get from using SSE4.2 instruction is likely eaten away
   by the indirect call in the PLT.  */
# define libc_hidden_builtin_def(name) \
	.globl __GI_strlen; __GI_strlen = __strlen_sse2
#endif

#include "../strlen.S"
