/* strlen SSE2 without bsf
   Copyright (C) 2010, 2011 Free Software Foundation, Inc.
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

/* only for strlen case we don't use optimized version for STATIC build just for SHARED */

#if (defined SHARED || defined USE_AS_STRCAT || defined USE_AS_STRNLEN) && !defined NOT_IN_libc

# ifndef USE_AS_STRCAT

#  include <sysdep.h>

#  define RETURN	ret

#  ifndef STRLEN
#   define STRLEN	__strlen_sse2_no_bsf
#  endif

	atom_text_section
ENTRY (STRLEN)
# endif
	xor	%eax, %eax
#  ifdef USE_AS_STRNLEN
	mov	%rsi, %r8
	sub	$4, %rsi
	jbe	L(len_less4_prolog)
#  endif
	cmpb	$0, (%rdi)
	jz	L(exit_tail0)
	cmpb	$0, 1(%rdi)
	jz	L(exit_tail1)
	cmpb	$0, 2(%rdi)
	jz	L(exit_tail2)
	cmpb	$0, 3(%rdi)
	jz	L(exit_tail3)

# ifdef USE_AS_STRNLEN
	sub	$4, %rsi
	jbe	L(len_less8_prolog)
# endif

	cmpb	$0, 4(%rdi)
	jz	L(exit_tail4)
	cmpb	$0, 5(%rdi)
	jz	L(exit_tail5)
	cmpb	$0, 6(%rdi)
	jz	L(exit_tail6)
	cmpb	$0, 7(%rdi)
	jz	L(exit_tail7)

# ifdef USE_AS_STRNLEN
	sub	$4, %rsi
	jbe	L(len_less12_prolog)
# endif

	cmpb	$0, 8(%rdi)
	jz	L(exit_tail8)
	cmpb	$0, 9(%rdi)
	jz	L(exit_tail9)
	cmpb	$0, 10(%rdi)
	jz	L(exit_tail10)
	cmpb	$0, 11(%rdi)
	jz	L(exit_tail11)

# ifdef USE_AS_STRNLEN
	sub	$4, %rsi
	jbe	L(len_less16_prolog)
# endif

	cmpb	$0, 12(%rdi)
	jz	L(exit_tail12)
	cmpb	$0, 13(%rdi)
	jz	L(exit_tail13)
	cmpb	$0, 14(%rdi)
	jz	L(exit_tail14)
	cmpb	$0, 15(%rdi)
	jz	L(exit_tail15)
	pxor	%xmm0, %xmm0
	lea	16(%rdi), %rcx
	lea	16(%rdi), %rax
	and	$-16, %rax

# ifdef USE_AS_STRNLEN
	and	$15, %rdi
	add	%rdi, %rsi
	sub	$64, %rsi
	jbe	L(len_less64)
# endif

	pcmpeqb	(%rax), %xmm0
	pmovmskb %xmm0, %edx
	pxor	%xmm1, %xmm1
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm1
	pmovmskb %xmm1, %edx
	pxor	%xmm2, %xmm2
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm2
	pmovmskb %xmm2, %edx
	pxor	%xmm3, %xmm3
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm3
	pmovmskb %xmm3, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

# ifdef USE_AS_STRNLEN
	sub	$64, %rsi
	jbe	L(len_less64)
# endif

	pcmpeqb	(%rax), %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm1
	pmovmskb %xmm1, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm2
	pmovmskb %xmm2, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm3
	pmovmskb %xmm3, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

# ifdef USE_AS_STRNLEN
	sub	$64, %rsi
	jbe	L(len_less64)
# endif

	pcmpeqb	(%rax), %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm1
	pmovmskb %xmm1, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm2
	pmovmskb %xmm2, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm3
	pmovmskb %xmm3, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

# ifdef USE_AS_STRNLEN
	sub	$64, %rsi
	jbe	L(len_less64)
# endif

	pcmpeqb	(%rax), %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm1
	pmovmskb %xmm1, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm2
	pmovmskb %xmm2, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm3
	pmovmskb %xmm3, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

# ifdef USE_AS_STRNLEN
	mov	%rax, %rdx
	and	$63, %rdx
	add	%rdx, %rsi
# endif

	and	$-0x40, %rax

	.p2align 4
L(aligned_64):
# ifdef USE_AS_STRNLEN
	sub	$64, %rsi
	jbe	L(len_less64)
# endif
	pcmpeqb	(%rax), %xmm0
	pcmpeqb	16(%rax), %xmm1
	pcmpeqb	32(%rax), %xmm2
	pcmpeqb	48(%rax), %xmm3
	pmovmskb %xmm0, %edx
	pmovmskb %xmm1, %r11d
	pmovmskb %xmm2, %r10d
	pmovmskb %xmm3, %r9d
	or	%edx, %r9d
	or	%r11d, %r9d
	or	%r10d, %r9d
	lea	64(%rax), %rax
	jz	L(aligned_64)

	test	%edx, %edx
	jnz	L(aligned_64_exit_16)
	test	%r11d, %r11d
	jnz	L(aligned_64_exit_32)
	test	%r10d, %r10d
	jnz	L(aligned_64_exit_48)
L(aligned_64_exit_64):
	pmovmskb %xmm3, %edx
	jmp	L(aligned_64_exit)
L(aligned_64_exit_48):
	lea	-16(%rax), %rax
	mov	%r10d, %edx
	jmp	L(aligned_64_exit)
L(aligned_64_exit_32):
	lea	-32(%rax), %rax
	mov	%r11d, %edx
	jmp	L(aligned_64_exit)
L(aligned_64_exit_16):
	lea	-48(%rax), %rax
L(aligned_64_exit):
L(exit):
	sub	%rcx, %rax
	test	%dl, %dl
	jz	L(exit_high)
	test	$0x01, %dl
	jnz	L(exit_tail0)

	test	$0x02, %dl
	jnz	L(exit_tail1)

	test	$0x04, %dl
	jnz	L(exit_tail2)

	test	$0x08, %dl
	jnz	L(exit_tail3)

	test	$0x10, %dl
	jnz	L(exit_tail4)

	test	$0x20, %dl
	jnz	L(exit_tail5)

	test	$0x40, %dl
	jnz	L(exit_tail6)
	add	$7, %eax
L(exit_tail0):
	RETURN

L(exit_high):
	add	$8, %eax
	test	$0x01, %dh
	jnz	L(exit_tail0)

	test	$0x02, %dh
	jnz	L(exit_tail1)

	test	$0x04, %dh
	jnz	L(exit_tail2)

	test	$0x08, %dh
	jnz	L(exit_tail3)

	test	$0x10, %dh
	jnz	L(exit_tail4)

	test	$0x20, %dh
	jnz	L(exit_tail5)

	test	$0x40, %dh
	jnz	L(exit_tail6)
	add	$7, %eax
	RETURN

# ifdef USE_AS_STRNLEN

	.p2align 4
L(len_less64):
	pxor	%xmm0, %xmm0
	add	$64, %rsi

	pcmpeqb	(%rax), %xmm0
	pmovmskb %xmm0, %edx
	pxor	%xmm1, %xmm1
	lea	16(%rax), %rax
	test	%edx, %edx
	jnz	L(strnlen_exit)

	sub	$16, %rsi
	jbe	L(return_start_len)

	pcmpeqb	(%rax), %xmm1
	pmovmskb %xmm1, %edx
	lea	16(%rax), %rax
	test	%edx, %edx
	jnz	L(strnlen_exit)

	sub	$16, %rsi
	jbe	L(return_start_len)

	pcmpeqb	(%rax), %xmm0
	pmovmskb %xmm0, %edx
	lea	16(%rax), %rax
	test	%edx, %edx
	jnz	L(strnlen_exit)

	sub	$16, %rsi
	jbe	L(return_start_len)

	pcmpeqb	(%rax), %xmm1
	pmovmskb %xmm1, %edx
	lea	16(%rax), %rax
	test	%edx, %edx
	jnz	L(strnlen_exit)

	mov	%r8, %rax
	ret

	.p2align 4
L(strnlen_exit):
	sub	%rcx, %rax

	test	%dl, %dl
	jz	L(strnlen_exit_high)
	mov	%dl, %cl
	and	$15, %cl
	jz	L(strnlen_exit_8)
	test	$0x01, %dl
	jnz	L(exit_tail0)
	test	$0x02, %dl
	jnz	L(strnlen_exit_tail1)
	test	$0x04, %dl
	jnz	L(strnlen_exit_tail2)
	sub	$4, %rsi
	jb	L(return_start_len)
	lea	3(%eax), %eax
	ret

	.p2align 4
L(strnlen_exit_8):
	test	$0x10, %dl
	jnz	L(strnlen_exit_tail4)
	test	$0x20, %dl
	jnz	L(strnlen_exit_tail5)
	test	$0x40, %dl
	jnz	L(strnlen_exit_tail6)
	sub	$8, %rsi
	jb	L(return_start_len)
	lea	7(%eax), %eax
	ret

	.p2align 4
L(strnlen_exit_high):
	mov	%dh, %ch
	and	$15, %ch
	jz	L(strnlen_exit_high_8)
	test	$0x01, %dh
	jnz	L(strnlen_exit_tail8)
	test	$0x02, %dh
	jnz	L(strnlen_exit_tail9)
	test	$0x04, %dh
	jnz	L(strnlen_exit_tail10)
	sub	$12, %rsi
	jb	L(return_start_len)
	lea	11(%eax), %eax
	ret

	.p2align 4
L(strnlen_exit_high_8):
	test	$0x10, %dh
	jnz	L(strnlen_exit_tail12)
	test	$0x20, %dh
	jnz	L(strnlen_exit_tail13)
	test	$0x40, %dh
	jnz	L(strnlen_exit_tail14)
	sub	$16, %rsi
	jb	L(return_start_len)
	lea	15(%eax), %eax
	ret

	.p2align 4
L(strnlen_exit_tail1):
	sub	$2, %rsi
	jb	L(return_start_len)
	lea	1(%eax), %eax
	ret

	.p2align 4
L(strnlen_exit_tail2):
	sub	$3, %rsi
	jb	L(return_start_len)
	lea	2(%eax), %eax
	ret

	.p2align 4
L(strnlen_exit_tail4):
	sub	$5, %rsi
	jb	L(return_start_len)
	lea	4(%eax), %eax
	ret

	.p2align 4
L(strnlen_exit_tail5):
	sub	$6, %rsi
	jb	L(return_start_len)
	lea	5(%eax), %eax
	ret

	.p2align 4
L(strnlen_exit_tail6):
	sub	$7, %rsi
	jb	L(return_start_len)
	lea	6(%eax), %eax
	ret

	.p2align 4
L(strnlen_exit_tail8):
	sub	$9, %rsi
	jb	L(return_start_len)
	lea	8(%eax), %eax
	ret

	.p2align 4
L(strnlen_exit_tail9):
	sub	$10, %rsi
	jb	L(return_start_len)
	lea	9(%eax), %eax
	ret

	.p2align 4
L(strnlen_exit_tail10):
	sub	$11, %rsi
	jb	L(return_start_len)
	lea	10(%eax), %eax
	ret

	.p2align 4
L(strnlen_exit_tail12):
	sub	$13, %rsi
	jb	L(return_start_len)
	lea	12(%eax), %eax
	ret

	.p2align 4
L(strnlen_exit_tail13):
	sub	$14, %rsi
	jb	L(return_start_len)
	lea	13(%eax), %eax
	ret

	.p2align 4
L(strnlen_exit_tail14):
	sub	$15, %rsi
	jb	L(return_start_len)
	lea	14(%eax), %eax
	ret

	.p2align 4
L(return_start_len):
	mov	%r8, %rax
	ret

/* for prolog only */

	.p2align 4
L(len_less4_prolog):
	add	$4, %rsi
	jz	L(exit_tail0)

	cmpb	$0, (%rdi)
	jz	L(exit_tail0)
	cmp	$1, %esi
	je	L(exit_tail1)

	cmpb	$0, 1(%rdi)
	jz	L(exit_tail1)
	cmp	$2, %esi
	je	L(exit_tail2)

	cmpb	$0, 2(%rdi)
	jz	L(exit_tail2)
	cmp	$3, %esi
	je	L(exit_tail3)

	cmpb	$0, 3(%rdi)
	jz	L(exit_tail3)
	mov	$4, %eax
	ret

	.p2align 4
L(len_less8_prolog):
	add	$4, %rsi

	cmpb	$0, 4(%rdi)
	jz	L(exit_tail4)
	cmp	$1, %esi
	je	L(exit_tail5)

	cmpb	$0, 5(%rdi)
	jz	L(exit_tail5)
	cmp	$2, %esi
	je	L(exit_tail6)

	cmpb	$0, 6(%rdi)
	jz	L(exit_tail6)
	cmp	$3, %esi
	je	L(exit_tail7)

	cmpb	$0, 7(%rdi)
	jz	L(exit_tail7)
	mov	$8, %eax
	ret

	.p2align 4
L(len_less12_prolog):
	add	$4, %rsi

	cmpb	$0, 8(%rdi)
	jz	L(exit_tail8)
	cmp	$1, %esi
	je	L(exit_tail9)

	cmpb	$0, 9(%rdi)
	jz	L(exit_tail9)
	cmp	$2, %esi
	je	L(exit_tail10)

	cmpb	$0, 10(%rdi)
	jz	L(exit_tail10)
	cmp	$3, %esi
	je	L(exit_tail11)

	cmpb	$0, 11(%rdi)
	jz	L(exit_tail11)
	mov	$12, %eax
	ret

	.p2align 4
L(len_less16_prolog):
	add	$4, %rsi

	cmpb	$0, 12(%rdi)
	jz	L(exit_tail12)
	cmp	$1, %esi
	je	L(exit_tail13)

	cmpb	$0, 13(%rdi)
	jz	L(exit_tail13)
	cmp	$2, %esi
	je	L(exit_tail14)

	cmpb	$0, 14(%rdi)
	jz	L(exit_tail14)
	cmp	$3, %esi
	je	L(exit_tail15)

	cmpb	$0, 15(%rdi)
	jz	L(exit_tail15)
	mov	$16, %eax
	ret
# endif

	.p2align 4
L(exit_tail1):
	add	$1, %eax
	RETURN

	.p2align 4
L(exit_tail2):
	add	$2, %eax
	RETURN

	.p2align 4
L(exit_tail3):
	add	$3, %eax
	RETURN

	.p2align 4
L(exit_tail4):
	add	$4, %eax
	RETURN

	.p2align 4
L(exit_tail5):
	add	$5, %eax
	RETURN

	.p2align 4
L(exit_tail6):
	add	$6, %eax
	RETURN

	.p2align 4
L(exit_tail7):
	add	$7, %eax
	RETURN

	.p2align 4
L(exit_tail8):
	add	$8, %eax
	RETURN

	.p2align 4
L(exit_tail9):
	add	$9, %eax
	RETURN

	.p2align 4
L(exit_tail10):
	add	$10, %eax
	RETURN

	.p2align 4
L(exit_tail11):
	add	$11, %eax
	RETURN

	.p2align 4
L(exit_tail12):
	add	$12, %eax
	RETURN

	.p2align 4
L(exit_tail13):
	add	$13, %eax
	RETURN

	.p2align 4
L(exit_tail14):
	add	$14, %eax
	RETURN

	.p2align 4
L(exit_tail15):
	add	$15, %eax
# ifndef USE_AS_STRCAT
	RETURN
END (STRLEN)
# endif
#endif
