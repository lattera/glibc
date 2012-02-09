/* strlen(str) -- determine the length of the string STR.
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

#include <sysdep.h>


	.text
ENTRY(strlen)
	xor	%rax, %rax
	mov	%edi, %ecx
	and	$0x3f, %ecx
	pxor	%xmm0, %xmm0
	cmp	$0x30, %ecx
	ja	L(next)
	movdqu	(%rdi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	L(exit_less16)
	mov	%rdi, %rax
	and	$-16, %rax
	jmp	L(align16_start)
L(next):
	mov	%rdi, %rax
	and	$-16, %rax
	pcmpeqb	(%rax), %xmm0
	mov	$-1, %esi
	sub	%rax, %rcx
	shl	%cl, %esi
	pmovmskb %xmm0, %edx
	and	%esi, %edx
	jnz	L(exit)
L(align16_start):
	pxor	%xmm0, %xmm0
	pxor	%xmm1, %xmm1
	pxor	%xmm2, %xmm2
	pxor	%xmm3, %xmm3
	.p2align 4
L(align16_loop):
	pcmpeqb	16(%rax), %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	L(exit16)

	pcmpeqb	32(%rax), %xmm1
	pmovmskb %xmm1, %edx
	test	%edx, %edx
	jnz	L(exit32)

	pcmpeqb	48(%rax), %xmm2
	pmovmskb %xmm2, %edx
	test	%edx, %edx
	jnz	L(exit48)

	pcmpeqb	64(%rax), %xmm3
	pmovmskb %xmm3, %edx
	lea	64(%rax), %rax
	test	%edx, %edx
	jz	L(align16_loop)
L(exit):
	sub	%rdi, %rax
L(exit_less16):
	bsf	%rdx, %rdx
	add	%rdx, %rax
	ret
	.p2align 4
L(exit16):
	sub	%rdi, %rax
	bsf	%rdx, %rdx
	lea	16(%rdx,%rax), %rax
	ret
	.p2align 4
L(exit32):
	sub	%rdi, %rax
	bsf	%rdx, %rdx
	lea	32(%rdx,%rax), %rax
	ret
	.p2align 4
L(exit48):
	sub	%rdi, %rax
	bsf	%rdx, %rdx
	lea	48(%rdx,%rax), %rax
	ret
END(strlen)
libc_hidden_builtin_def (strlen)
