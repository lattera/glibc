/* SSE2 version of strlen.
   Copyright (C) 2012-2015 Free Software Foundation, Inc.
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

/* Long lived register in strlen(s), strnlen(s, n) are:

	%xmm11 - zero
	%rdi   - s
	%r10  (s+n) & (~(64-1))
	%r11   s+n
*/


.text
ENTRY(strlen)

/* Test 64 bytes from %rax for zero. Save result as bitmask in %rdx.  */
#define FIND_ZERO	\
	pcmpeqb	(%rax), %xmm8;	\
	pcmpeqb	16(%rax), %xmm9;	\
	pcmpeqb	32(%rax), %xmm10;	\
	pcmpeqb	48(%rax), %xmm11;	\
	pmovmskb	%xmm8, %esi;	\
	pmovmskb	%xmm9, %edx;	\
	pmovmskb	%xmm10, %r8d;	\
	pmovmskb	%xmm11, %ecx;	\
	salq	$16, %rdx;	\
	salq	$16, %rcx;	\
	orq	%rsi, %rdx;	\
	orq	%r8, %rcx;	\
	salq	$32, %rcx;	\
	orq	%rcx, %rdx;

#ifdef AS_STRNLEN
/* Do not read anything when n==0.  */
	test	%rsi, %rsi
	jne	L(n_nonzero)
	xor	%rax, %rax
	ret
L(n_nonzero):

/* Initialize long lived registers.  */

	add	%rdi, %rsi
	mov	%rsi, %r10
	and	$-64, %r10
	mov	%rsi, %r11
#endif

	pxor	%xmm8, %xmm8
	pxor	%xmm9, %xmm9
	pxor	%xmm10, %xmm10
	pxor	%xmm11, %xmm11
	movq	%rdi, %rax
	movq	%rdi, %rcx
	andq	$4095, %rcx
/* Offsets 4032-4047 will be aligned into 4032 thus fit into page.  */
	cmpq	$4047, %rcx
/* We cannot unify this branching as it would be ~6 cycles slower.  */
	ja	L(cross_page)

#ifdef AS_STRNLEN
/* Test if end is among first 64 bytes.  */
# define STRNLEN_PROLOG	\
	mov	%r11, %rsi;	\
	subq	%rax, %rsi;	\
	andq	$-64, %rax;	\
	testq	$-64, %rsi;	\
	je	L(strnlen_ret)
#else
# define STRNLEN_PROLOG  andq $-64, %rax;
#endif

/* Ignore bits in mask that come before start of string.  */
#define PROLOG(lab)	\
	movq	%rdi, %rcx;	\
	xorq	%rax, %rcx;	\
	STRNLEN_PROLOG;	\
	sarq	%cl, %rdx;	\
	test	%rdx, %rdx;	\
	je	L(lab);	\
	bsfq	%rdx, %rax;	\
	ret

#ifdef AS_STRNLEN
	andq	$-16, %rax
	FIND_ZERO
#else
	/* Test first 16 bytes unaligned.  */
	movdqu	(%rax), %xmm12
	pcmpeqb	%xmm8, %xmm12
	pmovmskb	%xmm12, %edx
	test	%edx, %edx
	je 	L(next48_bytes)
	bsf	%edx, %eax /* If eax is zeroed 16bit bsf can be used.  */
	ret

L(next48_bytes):
/* Same as FIND_ZERO except we do not check first 16 bytes.  */
	andq	$-16, %rax
	pcmpeqb 16(%rax), %xmm9
	pcmpeqb 32(%rax), %xmm10
	pcmpeqb 48(%rax), %xmm11
	pmovmskb	%xmm9, %edx
	pmovmskb	%xmm10, %r8d
	pmovmskb	%xmm11, %ecx
	salq	$16, %rdx
	salq	$16, %rcx
	orq	%r8, %rcx
	salq	$32, %rcx
	orq	%rcx, %rdx
#endif

	/* When no zero byte is found xmm9-11 are zero so we do not have to
	   zero them.  */
	PROLOG(loop)

	.p2align 4
L(cross_page):
	andq	$-64, %rax
	FIND_ZERO
	PROLOG(loop_init)

#ifdef AS_STRNLEN
/* We must do this check to correctly handle strnlen (s, -1).  */
L(strnlen_ret):
	bts	%rsi, %rdx
	sarq	%cl, %rdx
	test	%rdx, %rdx
	je	L(loop_init)
	bsfq	%rdx, %rax
	ret
#endif
	.p2align 4
L(loop_init):
	pxor	%xmm9, %xmm9
	pxor	%xmm10, %xmm10
	pxor	%xmm11, %xmm11
#ifdef AS_STRNLEN
	.p2align 4
L(loop):

	addq	$64, %rax
	cmpq	%rax, %r10
	je	L(exit_end)

	movdqa	(%rax), %xmm8
	pminub	16(%rax), %xmm8
	pminub	32(%rax), %xmm8
	pminub	48(%rax), %xmm8
	pcmpeqb	%xmm11, %xmm8
	pmovmskb	%xmm8, %edx
	testl	%edx, %edx
	jne	L(exit)
	jmp	L(loop)

	.p2align 4
L(exit_end):
	cmp	%rax, %r11
	je	L(first) /* Do not read when end is at page boundary.  */
	pxor	%xmm8, %xmm8
	FIND_ZERO

L(first):
	bts	%r11, %rdx
	bsfq	%rdx, %rdx
	addq	%rdx, %rax
	subq	%rdi, %rax
	ret

	.p2align 4
L(exit):
	pxor	%xmm8, %xmm8
	FIND_ZERO

	bsfq	%rdx, %rdx
	addq	%rdx, %rax
	subq	%rdi, %rax
	ret

#else

	/* Main loop.  Unrolled twice to improve L2 cache performance on core2.  */
	.p2align 4
L(loop):

	movdqa	64(%rax), %xmm8
	pminub	80(%rax), %xmm8
	pminub	96(%rax), %xmm8
	pminub	112(%rax), %xmm8
	pcmpeqb	%xmm11, %xmm8
	pmovmskb	%xmm8, %edx
	testl	%edx, %edx
	jne	L(exit64)

	subq	$-128, %rax

	movdqa	(%rax), %xmm8
	pminub	16(%rax), %xmm8
	pminub	32(%rax), %xmm8
	pminub	48(%rax), %xmm8
	pcmpeqb	%xmm11, %xmm8
	pmovmskb	%xmm8, %edx
	testl	%edx, %edx
	jne	L(exit0)
	jmp	L(loop)

	.p2align 4
L(exit64):
	addq	$64, %rax
L(exit0):
	pxor	%xmm8, %xmm8
	FIND_ZERO

	bsfq	%rdx, %rdx
	addq	%rdx, %rax
	subq	%rdi, %rax
	ret

#endif

END(strlen)
libc_hidden_builtin_def (strlen)
