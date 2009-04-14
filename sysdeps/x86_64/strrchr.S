/* strrchr (str, ch) -- Return pointer to last occurrence of CH in STR.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>


	.text
ENTRY (strrchr)
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
	xorl	%eax, %eax
	movl	%edx, %esi
	orl	%ecx, %esi
	jnz	1f

2:	movdqa	(%rdi), %xmm0
	leaq	16(%rdi), %rdi
	movdqa	%xmm0, %xmm3
	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm3
	pmovmskb %xmm0, %edx
	pmovmskb %xmm3, %ecx
	movl	%edx, %esi
	orl	%ecx, %esi
	jz	2b

1:	bsfl	%ecx, %r9d
	movl	$0xffffffff, %r8d
	movl	$31, %ecx
	jnz	5f

	bsrl	%edx, %edx
	jz	2b
	leaq	-16(%rdi,%rdx), %rax
	jmp	2b

5:	subl	%r9d, %ecx
	shrl	%cl, %r8d
	andl	%r8d, %edx
	bsrl	%edx, %edx
	jz	4f
	leaq	-16(%rdi,%rdx), %rax
4:	ret
END (strrchr)

weak_alias (strrchr, rindex)
libc_hidden_builtin_def (strrchr)
