/* Return the pointer to the end of string, using SSE4.2
   Copyright (C) 2009 Free Software Foundation, Inc.
   Contributed by Intel Corporation.
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
#include "asm-syntax.h"

	.section .text.sse4.2,"ax",@progbits
ENTRY (__strend_sse4)
	pxor	%xmm2, %xmm2
	movq	%rdi, %rcx
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
	ret

1:	bsfl	%edx, %eax
	addq	%rdi, %rax
	ret

END (__strend_sse4)
