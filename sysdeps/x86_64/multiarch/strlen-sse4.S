/* strlen with SSE4
   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
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

#if defined SHARED && !defined NOT_IN_libc

#include <sysdep.h>

	.section .text.sse4.2,"ax",@progbits
ENTRY (__strlen_sse42)
	pxor	%xmm1, %xmm1
	movl	%edi, %ecx
	movq	%rdi, %r8
	andq	$~15, %rdi
	xor	%edi, %ecx
	pcmpeqb	(%rdi), %xmm1
	pmovmskb %xmm1, %edx
	shrl	%cl, %edx
	shll	%cl, %edx
	andl	%edx, %edx
	jnz	L(less16bytes)
	pxor	%xmm1, %xmm1

	.p2align 4
L(more64bytes_loop):
	pcmpistri $0x08, 16(%rdi), %xmm1
	jz	L(more32bytes)

	pcmpistri $0x08, 32(%rdi), %xmm1
	jz	L(more48bytes)

	pcmpistri $0x08, 48(%rdi), %xmm1
	jz	L(more64bytes)

	add	$64, %rdi
	pcmpistri $0x08, (%rdi), %xmm1
	jnz	L(more64bytes_loop)
	leaq	(%rdi,%rcx), %rax
	subq	%r8, %rax
	ret

	.p2align 4
L(more32bytes):
	leaq	16(%rdi,%rcx, 1), %rax
	subq	%r8, %rax
	ret

	.p2align 4
L(more48bytes):
	leaq	32(%rdi,%rcx, 1), %rax
	subq	%r8, %rax
	ret

	.p2align 4
L(more64bytes):
	leaq	48(%rdi,%rcx, 1), %rax
	subq	%r8, %rax
	ret

	.p2align 4
L(less16bytes):
	subq	%r8, %rdi
	bsfl	%edx, %eax
	addq	%rdi, %rax
	ret

END (__strlen_sse42)

#endif
