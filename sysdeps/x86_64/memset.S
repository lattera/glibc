/* memset/bzero -- set memory area to CH/0
   Optimized version for x86-64.
   Copyright (C) 2002-2015 Free Software Foundation, Inc.
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
#if IS_IN (libc)
ENTRY(__bzero)
	movq	%rdi, %rax /* Set return value.  */
	movq	%rsi, %rdx /* Set n.  */
	pxor	%xmm8, %xmm8
	jmp	L(entry_from_bzero)
END(__bzero)
weak_alias (__bzero, bzero)

/* Like memset but takes additional parameter with return value.  */
ENTRY(__memset_tail)
	movq	%rcx, %rax /* Set return value.  */

	movd	%esi, %xmm8
	punpcklbw	%xmm8, %xmm8
	punpcklwd	%xmm8, %xmm8
	pshufd	$0, %xmm8, %xmm8

	jmp	L(entry_from_bzero)
END(__memset_tail)
#endif

#if defined PIC && IS_IN (libc)
ENTRY_CHK (__memset_chk)
	cmpq	%rdx, %rcx
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END_CHK (__memset_chk)
#endif

ENTRY (memset)
	movd	%esi, %xmm8
	movq	%rdi, %rax
	punpcklbw	%xmm8, %xmm8
	punpcklwd	%xmm8, %xmm8
	pshufd	$0, %xmm8, %xmm8
L(entry_from_bzero):
	cmpq	$64, %rdx
	ja	L(loop_start)
	cmpq	$16, %rdx
	jbe	L(less_16_bytes)
	cmpq	$32, %rdx
	movdqu	%xmm8, (%rdi)
	movdqu	%xmm8, -16(%rdi,%rdx)
	ja	L(between_32_64_bytes)
L(return):
	rep
	ret
	.p2align 4
L(between_32_64_bytes):
	movdqu	%xmm8, 16(%rdi)
	movdqu	%xmm8, -32(%rdi,%rdx)
	ret
	.p2align 4
L(loop_start):
	leaq	64(%rdi), %rcx
	movdqu	%xmm8, (%rdi)
	andq	$-64, %rcx
	movdqu	%xmm8, -16(%rdi,%rdx)
	movdqu	%xmm8, 16(%rdi)
	movdqu	%xmm8, -32(%rdi,%rdx)
	movdqu	%xmm8, 32(%rdi)
	movdqu	%xmm8, -48(%rdi,%rdx)
	movdqu	%xmm8, 48(%rdi)
	movdqu	%xmm8, -64(%rdi,%rdx)
	addq	%rdi, %rdx
	andq	$-64, %rdx
	cmpq	%rdx, %rcx
	je	L(return)
	.p2align 4
L(loop):
	movdqa	%xmm8, (%rcx)
	movdqa	%xmm8, 16(%rcx)
	movdqa	%xmm8, 32(%rcx)
	movdqa	%xmm8, 48(%rcx)
	addq	$64, %rcx
	cmpq	%rcx, %rdx
	jne	L(loop)
	rep
	ret
L(less_16_bytes):
	movq %xmm8, %rcx
	testb	$24, %dl
	jne	L(between8_16bytes)
	testb	$4, %dl
	jne	L(between4_7bytes)
	testb	$1, %dl
	je	L(odd_byte)
	movb	%cl, (%rdi)
L(odd_byte):
	testb	$2, %dl
	je	L(return)
	movw	%cx, -2(%rax,%rdx)
	ret
L(between4_7bytes):
	movl	%ecx, (%rdi)
	movl	%ecx, -4(%rdi,%rdx)
	ret
L(between8_16bytes):
	movq	%rcx, (%rdi)
	movq	%rcx, -8(%rdi,%rdx)
	ret

END (memset)
libc_hidden_builtin_def (memset)

#if defined PIC && IS_IN (libc) && !defined USE_MULTIARCH
strong_alias (__memset_chk, __memset_zero_constant_len_parameter)
	.section .gnu.warning.__memset_zero_constant_len_parameter
	.string "memset used with constant zero length parameter; this could be due to transposed parameters"
#endif
