/* Copyright (C) 2009 Free Software Foundation, Inc.
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


	.text
ENTRY (memchr)
	movd	%esi, %xmm1
	movq	%rdi, %rcx
	punpcklbw %xmm1, %xmm1
	andq	$~15, %rdi
	testq	%rdx, %rdx
	punpcklbw %xmm1, %xmm1
	jz	3f
	orl	$0xffffffff, %esi
	movdqa	(%rdi), %xmm0
	pshufd	$0, %xmm1, %xmm1
	subq	%rdi, %rcx
	pcmpeqb	%xmm1, %xmm0
	addq	%rcx, %rdx
	shl	%cl, %esi
	pmovmskb %xmm0, %ecx
	andl	%esi, %ecx
	movl	$16, %esi
	jnz	1f
	cmpq	%rsi, %rdx
	jbe	3f

2:	movdqa	(%rdi,%rsi), %xmm0
	leaq	16(%rsi), %rsi
	pcmpeqb	%xmm1, %xmm0
	pmovmskb %xmm0, %ecx
	testl	%ecx, %ecx
	jnz	1f
	cmpq	%rsi, %rdx
	ja	2b

3:	xorl	%eax, %eax
	ret

1:	leaq	-16(%rdi,%rsi), %rax
	bsfl	%ecx, %ecx
	addq	%rcx, %rax
	leaq	-16(%rsi,%rcx), %rsi
	cmpq	%rsi, %rdx
	jbe	3b
	ret
END (memchr)

strong_alias (memchr, __memchr)
libc_hidden_builtin_def (memchr)
