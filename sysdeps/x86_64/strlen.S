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


	.text
ENTRY(strlen)
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

2:	movdqa	16(%rdi), %xmm0
	leaq	16(%rdi), %rdi
	pcmpeqb	%xmm1, %xmm0
	pmovmskb %xmm0, %edx
	testl	%edx, %edx
	jz	2b

1:	subq	%r8, %rdi
	bsfl	%edx, %eax
	addq	%rdi, %rax
	ret
END(strlen)
libc_hidden_builtin_def (strlen)
