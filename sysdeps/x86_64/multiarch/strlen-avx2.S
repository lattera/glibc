/* strlen/strnlen/wcslen/wcsnlen optimized with AVX2.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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

#if IS_IN (libc)

# include <sysdep.h>

# ifndef STRLEN
#  define STRLEN	__strlen_avx2
# endif

# ifdef USE_AS_WCSLEN
#  define VPCMPEQ	vpcmpeqd
#  define VPMINU	vpminud
# else
#  define VPCMPEQ	vpcmpeqb
#  define VPMINU	vpminub
# endif

# ifndef VZEROUPPER
#  define VZEROUPPER	vzeroupper
# endif

# define VEC_SIZE 32

	.section .text.avx,"ax",@progbits
ENTRY (STRLEN)
# ifdef USE_AS_STRNLEN
	/* Check for zero length.  */
	testq	%rsi, %rsi
	jz	L(zero)
#  ifdef USE_AS_WCSLEN
	shl	$2, %rsi
#  endif
	movq	%rsi, %r8
# endif
	movl	%edi, %ecx
	movq	%rdi, %rdx
	vpxor	%xmm0, %xmm0, %xmm0

	/* Check if we may cross page boundary with one vector load.  */
	andl	$(2 * VEC_SIZE - 1), %ecx
	cmpl	$VEC_SIZE, %ecx
	ja	L(cros_page_boundary)

	/* Check the first VEC_SIZE bytes.  */
	VPCMPEQ (%rdi), %ymm0, %ymm1
	vpmovmskb %ymm1, %eax
	testl	%eax, %eax

# ifdef USE_AS_STRNLEN
	jnz	L(first_vec_x0_check)
	/* Adjust length and check the end of data.  */
	subq	$VEC_SIZE, %rsi
	jbe	L(max)
# else
	jnz	L(first_vec_x0)
# endif

	/* Align data for aligned loads in the loop.  */
	addq	$VEC_SIZE, %rdi
	andl	$(VEC_SIZE - 1), %ecx
	andq	$-VEC_SIZE, %rdi

# ifdef USE_AS_STRNLEN
	/* Adjust length.  */
	addq	%rcx, %rsi

	subq	$(VEC_SIZE * 4), %rsi
	jbe	L(last_4x_vec_or_less)
# endif
	jmp	L(more_4x_vec)

	.p2align 4
L(cros_page_boundary):
	andl	$(VEC_SIZE - 1), %ecx
	andq	$-VEC_SIZE, %rdi
	VPCMPEQ (%rdi), %ymm0, %ymm1
	vpmovmskb %ymm1, %eax
	/* Remove the leading bytes.  */
	sarl	%cl, %eax
	testl	%eax, %eax
	jz	L(aligned_more)
	tzcntl	%eax, %eax
# ifdef USE_AS_STRNLEN
	/* Check the end of data.  */
	cmpq	%rax, %rsi
	jbe	L(max)
# endif
	addq	%rdi, %rax
	addq	%rcx, %rax
	subq	%rdx, %rax
# ifdef USE_AS_WCSLEN
	shrq	$2, %rax
# endif
	VZEROUPPER
	ret

	.p2align 4
L(aligned_more):
# ifdef USE_AS_STRNLEN
        /* "rcx" is less than VEC_SIZE.  Calculate "rdx + rcx - VEC_SIZE"
	    with "rdx - (VEC_SIZE - rcx)" instead of "(rdx + rcx) - VEC_SIZE"
	    to void possible addition overflow.  */
	negq	%rcx
	addq	$VEC_SIZE, %rcx

	/* Check the end of data.  */
	subq	%rcx, %rsi
	jbe	L(max)
# endif

	addq	$VEC_SIZE, %rdi

# ifdef USE_AS_STRNLEN
	subq	$(VEC_SIZE * 4), %rsi
	jbe	L(last_4x_vec_or_less)
# endif

L(more_4x_vec):
	/* Check the first 4 * VEC_SIZE.  Only one VEC_SIZE at a time
	   since data is only aligned to VEC_SIZE.  */
	VPCMPEQ (%rdi), %ymm0, %ymm1
	vpmovmskb %ymm1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x0)

	VPCMPEQ VEC_SIZE(%rdi), %ymm0, %ymm1
	vpmovmskb %ymm1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x1)

	VPCMPEQ (VEC_SIZE * 2)(%rdi), %ymm0, %ymm1
	vpmovmskb %ymm1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x2)

	VPCMPEQ (VEC_SIZE * 3)(%rdi), %ymm0, %ymm1
	vpmovmskb %ymm1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x3)

	addq	$(VEC_SIZE * 4), %rdi

# ifdef USE_AS_STRNLEN
	subq	$(VEC_SIZE * 4), %rsi
	jbe	L(last_4x_vec_or_less)
# endif

	/* Align data to 4 * VEC_SIZE.  */
	movq	%rdi, %rcx
	andl	$(4 * VEC_SIZE - 1), %ecx
	andq	$-(4 * VEC_SIZE), %rdi

# ifdef USE_AS_STRNLEN
	/* Adjust length.  */
	addq	%rcx, %rsi
# endif

	.p2align 4
L(loop_4x_vec):
	/* Compare 4 * VEC at a time forward.  */
	vmovdqa (%rdi), %ymm1
	vmovdqa	VEC_SIZE(%rdi), %ymm2
	vmovdqa	(VEC_SIZE * 2)(%rdi), %ymm3
	vmovdqa	(VEC_SIZE * 3)(%rdi), %ymm4
	VPMINU	%ymm1, %ymm2, %ymm5
	VPMINU	%ymm3, %ymm4, %ymm6
	VPMINU	%ymm5, %ymm6, %ymm5

	VPCMPEQ	%ymm5, %ymm0, %ymm5
	vpmovmskb %ymm5, %eax
	testl	%eax, %eax
	jnz	L(4x_vec_end)

	addq	$(VEC_SIZE * 4), %rdi

# ifndef USE_AS_STRNLEN
	jmp	L(loop_4x_vec)
# else
	subq	$(VEC_SIZE * 4), %rsi
	ja	L(loop_4x_vec)

L(last_4x_vec_or_less):
	/* Less than 4 * VEC and aligned to VEC_SIZE.  */
	addl	$(VEC_SIZE * 2), %esi
	jle	L(last_2x_vec)

	VPCMPEQ (%rdi), %ymm0, %ymm1
	vpmovmskb %ymm1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x0)

	VPCMPEQ VEC_SIZE(%rdi), %ymm0, %ymm1
	vpmovmskb %ymm1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x1)

	VPCMPEQ (VEC_SIZE * 2)(%rdi), %ymm0, %ymm1
	vpmovmskb %ymm1, %eax
	testl	%eax, %eax

	jnz	L(first_vec_x2_check)
	subl	$VEC_SIZE, %esi
	jle	L(max)

	VPCMPEQ (VEC_SIZE * 3)(%rdi), %ymm0, %ymm1
	vpmovmskb %ymm1, %eax
	testl	%eax, %eax

	jnz	L(first_vec_x3_check)
	movq	%r8, %rax
#  ifdef USE_AS_WCSLEN
	shrq	$2, %rax
#  endif
	VZEROUPPER
	ret

	.p2align 4
L(last_2x_vec):
	addl	$(VEC_SIZE * 2), %esi
	VPCMPEQ (%rdi), %ymm0, %ymm1
	vpmovmskb %ymm1, %eax
	testl	%eax, %eax

	jnz	L(first_vec_x0_check)
	subl	$VEC_SIZE, %esi
	jle	L(max)

	VPCMPEQ VEC_SIZE(%rdi), %ymm0, %ymm1
	vpmovmskb %ymm1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x1_check)
	movq	%r8, %rax
#  ifdef USE_AS_WCSLEN
	shrq	$2, %rax
#  endif
	VZEROUPPER
	ret

	.p2align 4
L(first_vec_x0_check):
	tzcntl	%eax, %eax
	/* Check the end of data.  */
	cmpq	%rax, %rsi
	jbe	L(max)
	addq	%rdi, %rax
	subq	%rdx, %rax
#  ifdef USE_AS_WCSLEN
	shrq	$2, %rax
#  endif
	VZEROUPPER
	ret

	.p2align 4
L(first_vec_x1_check):
	tzcntl	%eax, %eax
	/* Check the end of data.  */
	cmpq	%rax, %rsi
	jbe	L(max)
	addq	$VEC_SIZE, %rax
	addq	%rdi, %rax
	subq	%rdx, %rax
#  ifdef USE_AS_WCSLEN
	shrq	$2, %rax
#  endif
	VZEROUPPER
	ret

	.p2align 4
L(first_vec_x2_check):
	tzcntl	%eax, %eax
	/* Check the end of data.  */
	cmpq	%rax, %rsi
	jbe	L(max)
	addq	$(VEC_SIZE * 2), %rax
	addq	%rdi, %rax
	subq	%rdx, %rax
#  ifdef USE_AS_WCSLEN
	shrq	$2, %rax
#  endif
	VZEROUPPER
	ret

	.p2align 4
L(first_vec_x3_check):
	tzcntl	%eax, %eax
	/* Check the end of data.  */
	cmpq	%rax, %rsi
	jbe	L(max)
	addq	$(VEC_SIZE * 3), %rax
	addq	%rdi, %rax
	subq	%rdx, %rax
#  ifdef USE_AS_WCSLEN
	shrq	$2, %rax
#  endif
	VZEROUPPER
	ret

	.p2align 4
L(max):
	movq	%r8, %rax
#  ifdef USE_AS_WCSLEN
	shrq	$2, %rax
#  endif
	VZEROUPPER
	ret

	.p2align 4
L(zero):
	xorl	%eax, %eax
	ret
# endif

	.p2align 4
L(first_vec_x0):
	tzcntl	%eax, %eax
	addq	%rdi, %rax
	subq	%rdx, %rax
# ifdef USE_AS_WCSLEN
	shrq	$2, %rax
# endif
	VZEROUPPER
	ret

	.p2align 4
L(first_vec_x1):
	tzcntl	%eax, %eax
	addq	$VEC_SIZE, %rax
	addq	%rdi, %rax
	subq	%rdx, %rax
# ifdef USE_AS_WCSLEN
	shrq	$2, %rax
# endif
	VZEROUPPER
	ret

	.p2align 4
L(first_vec_x2):
	tzcntl	%eax, %eax
	addq	$(VEC_SIZE * 2), %rax
	addq	%rdi, %rax
	subq	%rdx, %rax
# ifdef USE_AS_WCSLEN
	shrq	$2, %rax
# endif
	VZEROUPPER
	ret

	.p2align 4
L(4x_vec_end):
	VPCMPEQ	%ymm1, %ymm0, %ymm1
	vpmovmskb %ymm1, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x0)
	VPCMPEQ %ymm2, %ymm0, %ymm2
	vpmovmskb %ymm2, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x1)
	VPCMPEQ %ymm3, %ymm0, %ymm3
	vpmovmskb %ymm3, %eax
	testl	%eax, %eax
	jnz	L(first_vec_x2)
	VPCMPEQ %ymm4, %ymm0, %ymm4
	vpmovmskb %ymm4, %eax
	testl	%eax, %eax
L(first_vec_x3):
	tzcntl	%eax, %eax
	addq	$(VEC_SIZE * 3), %rax
	addq	%rdi, %rax
	subq	%rdx, %rax
# ifdef USE_AS_WCSLEN
	shrq	$2, %rax
# endif
	VZEROUPPER
	ret

END (STRLEN)
#endif
