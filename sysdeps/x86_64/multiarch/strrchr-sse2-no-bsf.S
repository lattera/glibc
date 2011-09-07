/* strrchr with SSE2 without bsf and bsr
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

#if defined SHARED && !defined NOT_IN_libc

# include <sysdep.h>
# include "asm-syntax.h"

	atom_text_section
ENTRY (__strrchr_sse2_no_bsf)

	movd	%rsi, %xmm1
	pxor	%xmm2, %xmm2
	mov	%rdi, %rcx
	punpcklbw %xmm1, %xmm1
	punpcklbw %xmm1, %xmm1
	/* ECX has OFFSET. */
	and	$63, %rcx
	cmp	$48, %rcx
	pshufd	$0, %xmm1, %xmm1
	ja	L(crosscache)

/* unaligned string. */
	movdqu	(%rdi), %xmm0
	pcmpeqb	%xmm0, %xmm2
	pcmpeqb	%xmm1, %xmm0
	/* Find where NULL is.  */
	pmovmskb %xmm2, %rcx
	/* Check if there is a match.  */
	pmovmskb %xmm0, %rax
	add	$16, %rdi

	test	%rax, %rax
	jnz	L(unaligned_match1)

	test	%rcx, %rcx
	jnz	L(return_null)

	and	$-16, %rdi
	xor	%r8, %r8
	jmp	L(loop)

	.p2align 4
L(unaligned_match1):
	test	%rcx, %rcx
	jnz	L(prolog_find_zero_1)

	mov	%rax, %r8
	mov	%rdi, %rsi
	and	$-16, %rdi
	jmp	L(loop)

	.p2align 4
L(crosscache):
/* Hancle unaligned string.  */
	and	$15, %rcx
	and	$-16, %rdi
	pxor	%xmm3, %xmm3
	movdqa	(%rdi), %xmm0
	pcmpeqb	%xmm0, %xmm3
	pcmpeqb	%xmm1, %xmm0
	/* Find where NULL is.  */
	pmovmskb %xmm3, %rdx
	/* Check if there is a match.  */
	pmovmskb %xmm0, %rax
	/* Remove the leading bytes.  */
	shr	%cl, %rdx
	shr	%cl, %rax
	add	$16, %rdi

	test	%rax, %rax
	jnz	L(unaligned_match)

	test	%rdx, %rdx
	jnz	L(return_null)

	xor	%r8, %r8
	jmp	L(loop)

	.p2align 4
L(unaligned_match):
	test	%rdx, %rdx
	jnz	L(prolog_find_zero)

	mov	%rax, %r8
	lea	(%rdi, %rcx), %rsi

/* Loop start on aligned string.  */
	.p2align 4
L(loop):
	movdqa	(%rdi), %xmm0
	pcmpeqb	%xmm0, %xmm2
	add	$16, %rdi
	pcmpeqb	%xmm1, %xmm0
	pmovmskb %xmm2, %rcx
	pmovmskb %xmm0, %rax
	or	%rax, %rcx
	jnz	L(matches)

	movdqa	(%rdi), %xmm0
	pcmpeqb	%xmm0, %xmm2
	add	$16, %rdi
	pcmpeqb	%xmm1, %xmm0
	pmovmskb %xmm2, %rcx
	pmovmskb %xmm0, %rax
	or	%rax, %rcx
	jnz	L(matches)

	movdqa	(%rdi), %xmm0
	pcmpeqb	%xmm0, %xmm2
	add	$16, %rdi
	pcmpeqb	%xmm1, %xmm0
	pmovmskb %xmm2, %rcx
	pmovmskb %xmm0, %rax
	or	%rax, %rcx
	jnz	L(matches)

	movdqa	(%rdi), %xmm0
	pcmpeqb	%xmm0, %xmm2
	add	$16, %rdi
	pcmpeqb	%xmm1, %xmm0
	pmovmskb %xmm2, %rcx
	pmovmskb %xmm0, %rax
	or	%rax, %rcx
	jz	L(loop)

L(matches):
	test	%rax, %rax
	jnz	L(match)
L(return_value):
	test	%r8, %r8
	jz	L(return_null)
	mov	%r8, %rax
	mov	%rsi, %rdi
	jmp	L(match_exit)

	.p2align 4
L(match):
	pmovmskb %xmm2, %rcx
	test	%rcx, %rcx
	jnz	L(find_zero)
	mov	%rax, %r8
	mov	%rdi, %rsi
	jmp	L(loop)

	.p2align 4
L(find_zero):
	test	%cl, %cl
	jz	L(find_zero_high)
	mov	%cl, %dl
	and	$15, %dl
	jz	L(find_zero_8)
	test	$0x01, %cl
	jnz	L(FindZeroExit1)
	test	$0x02, %cl
	jnz	L(FindZeroExit2)
	test	$0x04, %cl
	jnz	L(FindZeroExit3)
	and	$1 << 4 - 1, %rax
	jz	L(return_value)
	jmp	L(match_exit)

	.p2align 4
L(find_zero_8):
	test	$0x10, %cl
	jnz	L(FindZeroExit5)
	test	$0x20, %cl
	jnz	L(FindZeroExit6)
	test	$0x40, %cl
	jnz	L(FindZeroExit7)
	and	$1 << 8 - 1, %rax
	jz	L(return_value)
	jmp	L(match_exit)

	.p2align 4
L(find_zero_high):
	mov	%ch, %dh
	and	$15, %dh
	jz	L(find_zero_high_8)
	test	$0x01, %ch
	jnz	L(FindZeroExit9)
	test	$0x02, %ch
	jnz	L(FindZeroExit10)
	test	$0x04, %ch
	jnz	L(FindZeroExit11)
	and	$1 << 12 - 1, %rax
	jz	L(return_value)
	jmp	L(match_exit)

	.p2align 4
L(find_zero_high_8):
	test	$0x10, %ch
	jnz	L(FindZeroExit13)
	test	$0x20, %ch
	jnz	L(FindZeroExit14)
	test	$0x40, %ch
	jnz	L(FindZeroExit15)
	and	$1 << 16 - 1, %rax
	jz	L(return_value)
	jmp	L(match_exit)

	.p2align 4
L(FindZeroExit1):
	and	$1, %rax
	jz	L(return_value)
	jmp	L(match_exit)

	.p2align 4
L(FindZeroExit2):
	and	$1 << 2 - 1, %rax
	jz	L(return_value)
	jmp	L(match_exit)

	.p2align 4
L(FindZeroExit3):
	and	$1 << 3 - 1, %rax
	jz	L(return_value)
	jmp	L(match_exit)

	.p2align 4
L(FindZeroExit5):
	and	$1 << 5 - 1, %rax
	jz	L(return_value)
	jmp	L(match_exit)

	.p2align 4
L(FindZeroExit6):
	and	$1 << 6 - 1, %rax
	jz	L(return_value)
	jmp	L(match_exit)

	.p2align 4
L(FindZeroExit7):
	and	$1 << 7 - 1, %rax
	jz	L(return_value)
	jmp	L(match_exit)

	.p2align 4
L(FindZeroExit9):
	and	$1 << 9 - 1, %rax
	jz	L(return_value)
	jmp	L(match_exit)

	.p2align 4
L(FindZeroExit10):
	and	$1 << 10 - 1, %rax
	jz	L(return_value)
	jmp	L(match_exit)

	.p2align 4
L(FindZeroExit11):
	and	$1 << 11 - 1, %rax
	jz	L(return_value)
	jmp	L(match_exit)

	.p2align 4
L(FindZeroExit13):
	and	$1 << 13 - 1, %rax
	jz	L(return_value)
	jmp	L(match_exit)

	.p2align 4
L(FindZeroExit14):
	and	$1 << 14 - 1, %rax
	jz	L(return_value)
	jmp	L(match_exit)

	.p2align 4
L(FindZeroExit15):
	and	$1 << 15 - 1, %rax
	jz	L(return_value)

	.p2align 4
L(match_exit):
	test	%ah, %ah
	jnz	L(match_exit_high)
	mov	%al, %dl
	and	$15 << 4, %dl
	jnz	L(match_exit_8)
	test	$0x08, %al
	jnz	L(Exit4)
	test	$0x04, %al
	jnz	L(Exit3)
	test	$0x02, %al
	jnz	L(Exit2)
	lea	-16(%rdi), %rax
	ret

	.p2align 4
L(match_exit_8):
	test	$0x80, %al
	jnz	L(Exit8)
	test	$0x40, %al
	jnz	L(Exit7)
	test	$0x20, %al
	jnz	L(Exit6)
	lea	-12(%rdi), %rax
	ret

	.p2align 4
L(match_exit_high):
	mov	%ah, %dh
	and	$15 << 4, %dh
	jnz	L(match_exit_high_8)
	test	$0x08, %ah
	jnz	L(Exit12)
	test	$0x04, %ah
	jnz	L(Exit11)
	test	$0x02, %ah
	jnz	L(Exit10)
	lea	-8(%rdi), %rax
	ret

	.p2align 4
L(match_exit_high_8):
	test	$0x80, %ah
	jnz	L(Exit16)
	test	$0x40, %ah
	jnz	L(Exit15)
	test	$0x20, %ah
	jnz	L(Exit14)
	lea	-4(%rdi), %rax
	ret

	.p2align 4
L(Exit2):
	lea	-15(%rdi), %rax
	ret

	.p2align 4
L(Exit3):
	lea	-14(%rdi), %rax
	ret

	.p2align 4
L(Exit4):
	lea	-13(%rdi), %rax
	ret

	.p2align 4
L(Exit6):
	lea	-11(%rdi), %rax
	ret

	.p2align 4
L(Exit7):
	lea	-10(%rdi), %rax
	ret

	.p2align 4
L(Exit8):
	lea	-9(%rdi), %rax
	ret

	.p2align 4
L(Exit10):
	lea	-7(%rdi), %rax
	ret

	.p2align 4
L(Exit11):
	lea	-6(%rdi), %rax
	ret

	.p2align 4
L(Exit12):
	lea	-5(%rdi), %rax
	ret

	.p2align 4
L(Exit14):
	lea	-3(%rdi), %rax
	ret

	.p2align 4
L(Exit15):
	lea	-2(%rdi), %rax
	ret

	.p2align 4
L(Exit16):
	lea	-1(%rdi), %rax
	ret

/* Return NULL.  */
	.p2align 4
L(return_null):
	xor	%rax, %rax
	ret

	.p2align 4
L(prolog_find_zero):
	add	%rcx, %rdi
	mov     %rdx, %rcx
L(prolog_find_zero_1):
	test	%cl, %cl
	jz	L(prolog_find_zero_high)
	mov	%cl, %dl
	and	$15, %dl
	jz	L(prolog_find_zero_8)
	test	$0x01, %cl
	jnz	L(PrologFindZeroExit1)
	test	$0x02, %cl
	jnz	L(PrologFindZeroExit2)
	test	$0x04, %cl
	jnz	L(PrologFindZeroExit3)
	and	$1 << 4 - 1, %rax
	jnz	L(match_exit)
	xor	%rax, %rax
	ret

	.p2align 4
L(prolog_find_zero_8):
	test	$0x10, %cl
	jnz	L(PrologFindZeroExit5)
	test	$0x20, %cl
	jnz	L(PrologFindZeroExit6)
	test	$0x40, %cl
	jnz	L(PrologFindZeroExit7)
	and	$1 << 8 - 1, %rax
	jnz	L(match_exit)
	xor	%rax, %rax
	ret

	.p2align 4
L(prolog_find_zero_high):
	mov	%ch, %dh
	and	$15, %dh
	jz	L(prolog_find_zero_high_8)
	test	$0x01, %ch
	jnz	L(PrologFindZeroExit9)
	test	$0x02, %ch
	jnz	L(PrologFindZeroExit10)
	test	$0x04, %ch
	jnz	L(PrologFindZeroExit11)
	and	$1 << 12 - 1, %rax
	jnz	L(match_exit)
	xor	%rax, %rax
	ret

	.p2align 4
L(prolog_find_zero_high_8):
	test	$0x10, %ch
	jnz	L(PrologFindZeroExit13)
	test	$0x20, %ch
	jnz	L(PrologFindZeroExit14)
	test	$0x40, %ch
	jnz	L(PrologFindZeroExit15)
	and	$1 << 16 - 1, %rax
	jnz	L(match_exit)
	xor	%rax, %rax
	ret

	.p2align 4
L(PrologFindZeroExit1):
	and	$1, %rax
	jnz	L(match_exit)
	xor	%rax, %rax
	ret

	.p2align 4
L(PrologFindZeroExit2):
	and	$1 << 2 - 1, %rax
	jnz	L(match_exit)
	xor	%rax, %rax
	ret

	.p2align 4
L(PrologFindZeroExit3):
	and	$1 << 3 - 1, %rax
	jnz	L(match_exit)
	xor	%rax, %rax
	ret

	.p2align 4
L(PrologFindZeroExit5):
	and	$1 << 5 - 1, %rax
	jnz	L(match_exit)
	xor	%rax, %rax
	ret

	.p2align 4
L(PrologFindZeroExit6):
	and	$1 << 6 - 1, %rax
	jnz	L(match_exit)
	xor	%rax, %rax
	ret

	.p2align 4
L(PrologFindZeroExit7):
	and	$1 << 7 - 1, %rax
	jnz	L(match_exit)
	xor	%rax, %rax
	ret

	.p2align 4
L(PrologFindZeroExit9):
	and	$1 << 9 - 1, %rax
	jnz	L(match_exit)
	xor	%rax, %rax
	ret

	.p2align 4
L(PrologFindZeroExit10):
	and	$1 << 10 - 1, %rax
	jnz	L(match_exit)
	xor	%rax, %rax
	ret

	.p2align 4
L(PrologFindZeroExit11):
	and	$1 << 11 - 1, %rax
	jnz	L(match_exit)
	xor	%rax, %rax
	ret

	.p2align 4
L(PrologFindZeroExit13):
	and	$1 << 13 - 1, %rax
	jnz	L(match_exit)
	xor	%rax, %rax
	ret

	.p2align 4
L(PrologFindZeroExit14):
	and	$1 << 14 - 1, %rax
	jnz	L(match_exit)
	xor	%rax, %rax
	ret

	.p2align 4
L(PrologFindZeroExit15):
	and	$1 << 15 - 1, %rax
	jnz	L(match_exit)
	xor	%rax, %rax
	ret

END (__strrchr_sse2_no_bsf)
#endif
