/* strnlen(str,maxlen) -- determine the length of the string STR up to MAXLEN.
   Copyright (C) 2010 Free Software Foundation, Inc.
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
ENTRY(__strnlen)
	movq	%rsi, %rax
	testq	%rsi, %rsi
	jz	3f
	pxor	%xmm2, %xmm2
	movq	%rdi, %rcx
	movq	%rdi, %r8
	movq	$16, %r9
	andq	$~15, %rdi
	movdqa	%xmm2, %xmm1
	pcmpeqb	(%rdi), %xmm2
	orl	$0xffffffff, %r10d
	subq	%rdi, %rcx
	shll	%cl, %r10d
	subq	%rcx, %r9
	pmovmskb %xmm2, %edx
	andl	%r10d, %edx
	jnz	1f
	subq	%r9, %rsi
	jbe	3f

2:	movdqa	16(%rdi), %xmm0
	leaq	16(%rdi), %rdi
	pcmpeqb	%xmm1, %xmm0
	pmovmskb %xmm0, %edx
	testl	%edx, %edx
	jnz	1f
	subq	$16, %rsi
	jnbe	2b
3:	ret

1:	subq	%r8, %rdi
	bsfl	%edx, %edx
	addq	%rdi, %rdx
	cmpq	%rdx, %rax
	cmovnbq	%rdx, %rax
	ret
END(__strnlen)
weak_alias (__strnlen, strnlen)
libc_hidden_def (strnlen)
