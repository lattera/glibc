/* strchr (str, ch) -- Return pointer to first occurrence of CH in STR.
   For AMD x86-64.
   Copyright (C) 2009 Free Software Foundation, Inc.
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
ENTRY (strchr)
	movd	%esi, %xmm1
	movq	%rdi, %rcx
	punpcklbw %xmm1, %xmm1
	andq	$~15, %rdi
	pxor	%xmm2, %xmm2
	punpcklbw %xmm1, %xmm1
	orl	$0xffffffff, %esi
	movdqa	(%rdi), %xmm0
	pshufd	$0, %xmm1, %xmm1
	subq	%rdi, %rcx
	movdqa	%xmm0, %xmm3
	leaq	16(%rdi), %rdi
	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm3
	shl	%cl, %esi
	pmovmskb %xmm0, %edx
	pmovmskb %xmm3, %ecx
	andl	%esi, %edx
	andl	%esi, %ecx
	orl	%edx, %ecx
	jnz	1f

2:	movdqa	(%rdi), %xmm0
	leaq	16(%rdi), %rdi
	movdqa	%xmm0, %xmm3
	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm3
	pmovmskb %xmm0, %edx
	pmovmskb %xmm3, %ecx
	orl	%edx, %ecx
	jz	2b

1:	bsfl	%edx, %edx
	jz	4f
	bsfl	%ecx, %ecx
	leaq	-16(%rdi,%rdx), %rax
	cmpl	%edx, %ecx
	je	5f
4:	xorl	%eax, %eax
5:	ret
END (strchr)

weak_alias (strchr, index)
libc_hidden_builtin_def (strchr)

