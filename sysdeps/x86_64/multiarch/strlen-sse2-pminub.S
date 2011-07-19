/* strlen SSE2
   Copyright (C) 2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#if !defined NOT_IN_libc && (defined SHARED || defined USE_AS_STRCAT)

# ifndef USE_AS_STRCAT

#  include <sysdep.h>

#  define RETURN ret

	.section .text.sse2,"ax",@progbits
ENTRY (__strlen_sse2_pminub)

# endif
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
	mov	$-1, %r10d
	sub	%rax, %rcx
	shl	%cl, %r10d
	pmovmskb %xmm0, %edx
	and	%r10d, %edx
	jnz	L(exit)
L(align16_start):
	pxor	%xmm0, %xmm0
	pxor	%xmm1, %xmm1
	pxor	%xmm2, %xmm2
	pxor	%xmm3, %xmm3
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
	test	%edx, %edx
	jnz	L(exit64)

	pcmpeqb	80(%rax), %xmm0
	add	$64, %rax
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
	test	%edx, %edx
	jnz	L(exit64)

	pcmpeqb	80(%rax), %xmm0
	add	$64, %rax
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
	test	%edx, %edx
	jnz	L(exit64)

	pcmpeqb	80(%rax), %xmm0
	add	$64, %rax
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
	test	%edx, %edx
	jnz	L(exit64)


	test	$0x3f, %rax
	jz	L(align64_loop)

	pcmpeqb	80(%rax), %xmm0
	add	$80, %rax
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	L(exit)

	test	$0x3f, %rax
	jz	L(align64_loop)

	pcmpeqb	16(%rax), %xmm1
	add	$16, %rax
	pmovmskb %xmm1, %edx
	test	%edx, %edx
	jnz	L(exit)

	test	$0x3f, %rax
	jz	L(align64_loop)

	pcmpeqb	16(%rax), %xmm2
	add	$16, %rax
	pmovmskb %xmm2, %edx
	test	%edx, %edx
	jnz	L(exit)

	test	$0x3f, %rax
	jz	L(align64_loop)

	pcmpeqb	16(%rax), %xmm3
	add	$16, %rax
	pmovmskb %xmm3, %edx
	test	%edx, %edx
	jnz	L(exit)

	add	$16, %rax
	.p2align 4
	L(align64_loop):
	movaps	(%rax),	%xmm4
	pminub	16(%rax),	%xmm4
	movaps	32(%rax),	%xmm5
	pminub	48(%rax),	%xmm5
	add	$64,	%rax
	pminub	%xmm4,	%xmm5
	pcmpeqb	%xmm0,	%xmm5
	pmovmskb %xmm5,	%edx
	test	%edx,	%edx
	jz	L(align64_loop)


	pcmpeqb	-64(%rax), %xmm0
	sub	$80,	%rax
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
	sub	%rdi, %rax
	bsf	%rdx, %rdx
	add	%rdx, %rax
	add	$64, %rax
	RETURN

	.p2align 4
L(exit):
	sub	%rdi, %rax
L(exit_less16):
	bsf	%rdx, %rdx
	add	%rdx, %rax
	RETURN
	.p2align 4
L(exit16):
	sub	%rdi, %rax
	bsf	%rdx, %rdx
	add	%rdx, %rax
	add	$16, %rax
	RETURN
	.p2align 4
L(exit32):
	sub	%rdi, %rax
	bsf	%rdx, %rdx
	add	%rdx, %rax
	add	$32, %rax
	RETURN
	.p2align 4
L(exit48):
	sub	%rdi, %rax
	bsf	%rdx, %rdx
	add	%rdx, %rax
	add	$48, %rax
	RETURN
	.p2align 4
L(exit64):
	sub	%rdi, %rax
	bsf	%rdx, %rdx
	add	%rdx, %rax
	add	$64, %rax
# ifndef USE_AS_STRCAT
	RETURN

END (__strlen_sse2_pminub)
# endif
#endif
