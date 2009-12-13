/* strcpy with SSSE3
   Copyright (C) 2009 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <init-arch.h>

#if !defined (USE_AS_STPCPY) && !defined (USE_AS_STRNCPY)
# ifndef STRCPY
#  define STRCPY strcpy
# endif
#endif

#ifdef USE_AS_STPCPY
# ifdef USE_AS_STRNCPY
#  define STRCPY_SSSE3	__stpncpy_ssse3
#  define STRCPY_SSE2	__stpncpy_sse2
#  define __GI_STRCPY	__GI_stpncpy
# else
#  define STRCPY_SSSE3	__stpcpy_ssse3
#  define STRCPY_SSE2	__stpcpy_sse2
#  define __GI_STRCPY	__GI_stpcpy
#  define __GI___STRCPY	__GI___stpcpy
# endif
#else
# ifdef USE_AS_STRNCPY
#  define STRCPY_SSSE3	__strncpy_ssse3
#  define STRCPY_SSE2	__strncpy_sse2
#  define __GI_STRCPY	__GI_strncpy
# else
#  define STRCPY_SSSE3	__strcpy_ssse3
#  define STRCPY_SSE2	__strcpy_sse2
#  define __GI_STRCPY	__GI_strcpy
# endif
#endif

#ifndef LABEL
#define LABEL(l) L(l)
#endif

/* Define multiple versions only for the definition in libc.  */
#ifndef NOT_IN_libc
	.text
ENTRY(STRCPY)
	.type	STRCPY, @gnu_indirect_function
	cmpl	$0, __cpu_features+KIND_OFFSET(%rip)
	jne	1f
	call	__init_cpu_features
1:	leaq	STRCPY_SSE2(%rip), %rax
	testl	$bit_SSSE3, __cpu_features+CPUID_OFFSET+index_SSSE3(%rip)
	jz	2f
	leaq	STRCPY_SSSE3(%rip), %rax
2:	ret
END(STRCPY)

	.section .text.ssse3,"ax",@progbits
STRCPY_SSSE3:
	cfi_startproc
	CALL_MCOUNT

/*
 * This implementation uses SSE to copy up to 16 bytes at a time.
 */
#ifdef USE_AS_STRNCPY
	test    %rdx, %rdx
	jz      LABEL(strncpy_exitz)
	mov     %rdx, %r8
#else
	xor	%edx, %edx
#endif
	mov	%esi, %ecx
	and	$0xfffffffffffffff0, %rsi	/*force rsi 16 byte align*/
	and	$15, %ecx
	mov	%rdi, %rax			/*store return parameter*/


	pxor	%xmm0, %xmm0			/* clear %xmm0 */
	pcmpeqb	(%rsi), %xmm0			/* compare 16 bytes in (%rsi) and %xmm0 for equality, try to find null char*/
	pmovmskb %xmm0, %edx			/* move each byte mask of %xmm0 to edx*/
	shr	%cl, %edx			/* get real bits left in edx*/
	test	%edx, %edx			/* edx must be 0 if there is no null char from rsi+%rcx */
	jnz	LABEL(less16bytes)

#ifdef USE_AS_STRNCPY
	lea	-16(%r8,%rcx), %r11
	cmp	$0, %r11
	jle	LABEL(less16bytes)		/* if r8 + rcx <= 16, branch to less16bytes.  */
#endif

	mov	%rcx, %r9
	or	%edi, %ecx
	and	$15, %ecx
	lea	-16(%r9), %r10
	jz	LABEL(ashr_0)			/* ecx must be 0 if offset of rsi and rdi is 16 byte align*/

	neg	%r10				/* store the rest in rsi aligned 16 bytes for unaligned_exit*/

	pxor	%xmm0, %xmm0			/* clear %xmm0, may be polluted by unaligned operation*/
	pcmpeqb	16(%rsi), %xmm0			/* compare 16 bytes in (%rsi) and %xmm0 for equality, try to find null char*/
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(less32bytes)
	/*
	* at least 16 byte available to fill destination rdi
	*/
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(less32bytes_strncpy_truncation)
#endif
	mov	(%rsi, %r9), %rdx
	mov	%rdx, (%rdi)
	mov	8(%rsi, %r9), %rdx
	mov	%rdx, 8(%rdi)

	/*
	* so far destatination rdi may be aligned by 16, re-calculate rsi to jump
	* crossponding case
	* rcx is offset of rsi
	* rax is offset of rdi
	*/

	and	$0xfffffffffffffff0, %rdi	/* force rdi 16 byte align */
	mov	%rax, %rdx			/* rax store orignal rdi */
	xor	%rdi, %rdx			/* equal to and $15, %rdx */
#ifdef USE_AS_STRNCPY
	add     %rdx, %r8
#endif

	add	$16, %rdi			/* next 16 bytes for rdi */
	sub	%rdx, %r9

	lea	16(%r9, %rsi), %rsi		/*re-calculate rsi by (16 - rdx)+ rcx */
	mov	%esi, %ecx			/*store offset of rsi */
	and	$0xfffffffffffffff0, %rsi	/* force rsi 16 byte align */

	and	$15, %ecx			/* ecx must be 0 if rdx is equal to rcx*/
	jz	LABEL(ashr_0)

	lea	-16(%rcx), %r10
	mov	%rcx, %r9
	neg	%r10
	lea	LABEL(unaligned_table)(%rip), %r11
	movslq  (%r11, %rcx,4), %rcx
	lea	(%r11, %rcx), %rcx
	jmp	*%rcx

 /*
 * The following cases will be handled by ashr_0 & ashr_0_start
 *  rcx(offset of rsi)  rax(offset of rdi)  relative offset  corresponding case
 *	0		    0		  0		 ashr_0
 *	n(1~15)	     n(1~15)	   0		 ashr_0_start
 *
 */
	.p2align 5
LABEL(ashr_0):
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_aligned)
#endif
	movdqa  (%rsi), %xmm1	   /* fetch first 16 bytes from rsi */
	movdqa  %xmm1, (%rdi)	   /* store first 16 bytes into rdi */
	add     $16, %rsi
	add     $16, %rdi
	pcmpeqb  (%rsi), %xmm0		   /* compare 16 bytes in (%rsi) and %xmm0 for equality, try to find null char */
	pmovmskb  %xmm0, %edx		   /* move each byte mask of %xmm0 to edx*/

	test    %edx, %edx		  /* edx must be 0 if there is no null char in rsi*/
	jnz	LABEL(aligned_16bytes)

LABEL(ashr_0_loop):
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_aligned)
#endif
	movdqa  (%rsi, %rcx), %xmm1
	movdqa  %xmm1, (%rdi, %rcx)
	add	$16, %rcx
	pcmpeqb  (%rsi, %rcx), %xmm0
	pmovmskb  %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(aligned_exit)

#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_aligned)
#endif
	movdqa  (%rsi, %rcx), %xmm1
	movdqa  %xmm1, (%rdi, %rcx)
	add	$16, %rcx
	pcmpeqb  (%rsi, %rcx), %xmm0
	pmovmskb  %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(aligned_exit)

#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_aligned)
#endif
	movdqa  (%rsi, %rcx), %xmm1
	movdqa  %xmm1, (%rdi, %rcx)
	add	$16, %rcx
	pcmpeqb  (%rsi, %rcx), %xmm0
	pmovmskb  %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(aligned_exit)

#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_aligned)
#endif
	movdqa  (%rsi, %rcx), %xmm1
	movdqa  %xmm1, (%rdi, %rcx)
	add	$16, %rcx
	pcmpeqb  (%rsi, %rcx), %xmm0
	pmovmskb  %xmm0, %edx
	test	%edx, %edx
	jz	LABEL(ashr_0_loop)

	jmp	LABEL(aligned_exit)
        .p2align 4

/*
 * The following cases will be handled by ashr_15
 *  rcx(offset of rsi)  rax(offset of rdi)	relative offset  	  corresponding case
 *      n(15)		n - 15		15((16 - (n -15) + n)%16	 ashr_15
 *
 * Based on above operation , start from  (%r9 + rsi) to the left of this cache bank, there is no null byte
 */
	.p2align 4
LABEL(ashr_15):
	xor	%ecx, %ecx				/*clear ecx */
#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	.p2align 4
LABEL(ashr_15_use_ssse3):
	movdqa	16(%rsi, %rcx), %xmm3
	pcmpeqb	%xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $15, (%rsi, %rcx), %xmm3
	movdqa	%xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	movdqa  16(%rsi, %rcx), %xmm3
	pcmpeqb %xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $15, (%rsi, %rcx), %xmm3
	movdqa  %xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif
	jmp	LABEL(ashr_15_use_ssse3)

/*
 * The following cases will be handled by ashr_14
 *  rcx(offset of rsi)  rax(offset of rdi)	relative offset  	  corresponding case
 *      n(14~15)		n - 14		14((16 - (n -14) + n)%16	 ashr_14
 *
 * Based on above operation , start from  (%r9 + rsi) to the left of this cache bank, there is no null byte
 */
	.p2align 4
LABEL(ashr_14):
	xor	%ecx, %ecx				/*clear ecx */
#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	.p2align 4
LABEL(ashr_14_use_ssse3):
	movdqa	16(%rsi, %rcx), %xmm3
	pcmpeqb	%xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $14, (%rsi, %rcx), %xmm3
	movdqa	%xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	movdqa  16(%rsi, %rcx), %xmm3
	pcmpeqb %xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $14, (%rsi, %rcx), %xmm3
	movdqa  %xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif
	jmp	LABEL(ashr_14_use_ssse3)

/*
 * The following cases will be handled by ashr_13
 *  rcx(offset of rsi)  rax(offset of rdi)	relative offset  	  corresponding case
 *      n(13~15)		n - 13		13((16 - (n -13) + n)%16	 ashr_13
 *
 * Based on above operation , start from  (%r9 + rsi) to the left of this cache bank, there is no null byte
 */
	.p2align 4
LABEL(ashr_13):
	xor	%ecx, %ecx				/*clear ecx */
#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	.p2align 4
LABEL(ashr_13_use_ssse3):
	movdqa	16(%rsi, %rcx), %xmm3
	pcmpeqb	%xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $13, (%rsi, %rcx), %xmm3
	movdqa	%xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	movdqa  16(%rsi, %rcx), %xmm3
	pcmpeqb %xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $13, (%rsi, %rcx), %xmm3
	movdqa  %xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif
	jmp	LABEL(ashr_13_use_ssse3)

/*
 * The following cases will be handled by ashr_12
 *  rcx(offset of rsi)  rax(offset of rdi)	relative offset  	  corresponding case
 *      n(12~15)		n - 12		12((16 - (n -12) + n)%16	 ashr_12
 *
 * Based on above operation , start from  (%r9 + rsi) to the left of this cache bank, there is no null byte
 */
	.p2align 4
LABEL(ashr_12):
	xor	%ecx, %ecx				/*clear ecx */
#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	.p2align 4
LABEL(ashr_12_use_ssse3):
	movdqa	16(%rsi, %rcx), %xmm3
	pcmpeqb	%xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $12, (%rsi, %rcx), %xmm3
	movdqa	%xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	movdqa  16(%rsi, %rcx), %xmm3
	pcmpeqb %xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $12, (%rsi, %rcx), %xmm3
	movdqa  %xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif
	jmp	LABEL(ashr_12_use_ssse3)

/*
 * The following cases will be handled by ashr_11
 *  rcx(offset of rsi)  rax(offset of rdi)	relative offset  	  corresponding case
 *      n(11~15)		n - 11		11((16 - (n -11) + n)%16	 ashr_11
 *
 * Based on above operation , start from  (%r9 + rsi) to the left of this cache bank, there is no null byte
 */
	.p2align 4
LABEL(ashr_11):
	xor	%ecx, %ecx				/*clear ecx */
#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	.p2align 4
LABEL(ashr_11_use_ssse3):
	movdqa	16(%rsi, %rcx), %xmm3
	pcmpeqb	%xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $11, (%rsi, %rcx), %xmm3
	movdqa	%xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	movdqa  16(%rsi, %rcx), %xmm3
	pcmpeqb %xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $11, (%rsi, %rcx), %xmm3
	movdqa  %xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif
	jmp	LABEL(ashr_11_use_ssse3)

/*
 * The following cases will be handled by ashr_10
 *  rcx(offset of rsi)  rax(offset of rdi)	relative offset  	  corresponding case
 *      n(10~15)		n - 10		10((16 - (n -10) + n)%16	 ashr_10
 *
 * Based on above operation , start from  (%r9 + rsi) to the left of this cache bank, there is no null byte
 */
	.p2align 4
LABEL(ashr_10):
	xor	%ecx, %ecx				/*clear ecx */
#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	.p2align 4
LABEL(ashr_10_use_ssse3):
	movdqa	16(%rsi, %rcx), %xmm3
	pcmpeqb	%xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $10, (%rsi, %rcx), %xmm3
	movdqa	%xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	movdqa  16(%rsi, %rcx), %xmm3
	pcmpeqb %xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $10, (%rsi, %rcx), %xmm3
	movdqa  %xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif
	jmp	LABEL(ashr_10_use_ssse3)

/*
 * The following cases will be handled by ashr_9
 *  rcx(offset of rsi)  rax(offset of rdi)	relative offset  	  corresponding case
 *      n(9~15)		n - 9		9((16 - (n -9) + n)%16	 ashr_9
 *
 * Based on above operation , start from  (%r9 + rsi) to the left of this cache bank, there is no null byte
 */
	.p2align 4
LABEL(ashr_9):
	xor	%ecx, %ecx				/*clear ecx */
#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	.p2align 4
LABEL(ashr_9_use_ssse3):
	movdqa	16(%rsi, %rcx), %xmm3
	pcmpeqb	%xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $9, (%rsi, %rcx), %xmm3
	movdqa	%xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	movdqa  16(%rsi, %rcx), %xmm3
	pcmpeqb %xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $9, (%rsi, %rcx), %xmm3
	movdqa  %xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif
	jmp	LABEL(ashr_9_use_ssse3)

/*
 * The following cases will be handled by ashr_8
 *  rcx(offset of rsi)  rax(offset of rdi)	relative offset  	  corresponding case
 *      n(8~15)		n - 8		8((16 - (n -8) + n)%16	 ashr_8
 *
 * Based on above operation , start from  (%r9 + rsi) to the left of this cache bank, there is no null byte
 */
	.p2align 4
LABEL(ashr_8):
	xor	%ecx, %ecx				/*clear ecx */
#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	.p2align 4
LABEL(ashr_8_use_ssse3):
	movdqa	16(%rsi, %rcx), %xmm3
	pcmpeqb	%xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $8, (%rsi, %rcx), %xmm3
	movdqa	%xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	movdqa  16(%rsi, %rcx), %xmm3
	pcmpeqb %xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $8, (%rsi, %rcx), %xmm3
	movdqa  %xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif
	jmp	LABEL(ashr_8_use_ssse3)

/*
 * The following cases will be handled by ashr_7
 *  rcx(offset of rsi)  rax(offset of rdi)	relative offset  	  corresponding case
 *      n(7~15)		n - 7		7((16 - (n -7) + n)%16	 ashr_7
 *
 * Based on above operation , start from  (%r9 + rsi) to the left of this cache bank, there is no null byte
 */
	.p2align 4
LABEL(ashr_7):
	xor	%ecx, %ecx				/*clear ecx */
#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif
	.p2align 4

LABEL(ashr_7_use_ssse3):
	movdqa	16(%rsi, %rcx), %xmm3
	pcmpeqb	%xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $7, (%rsi, %rcx), %xmm3
	movdqa	%xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	movdqa  16(%rsi, %rcx), %xmm3
	pcmpeqb %xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $7, (%rsi, %rcx), %xmm3
	movdqa  %xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif
	jmp	LABEL(ashr_7_use_ssse3)

/*
 * The following cases will be handled by ashr_6
 *  rcx(offset of rsi)  rax(offset of rdi)	relative offset  	  corresponding case
 *      n(6~15)		n - 6		6((16 - (n -6) + n)%16	 ashr_6
 *
 * Based on above operation , start from  (%r9 + rsi) to the left of this cache bank, there is no null byte
 */
	.p2align 4
LABEL(ashr_6):
	xor	%ecx, %ecx				/*clear ecx */
#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	.p2align 4
LABEL(ashr_6_use_ssse3):
	movdqa	16(%rsi, %rcx), %xmm3
	pcmpeqb	%xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $6, (%rsi, %rcx), %xmm3
	movdqa	%xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	movdqa  16(%rsi, %rcx), %xmm3
	pcmpeqb %xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $6, (%rsi, %rcx), %xmm3
	movdqa  %xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif
	jmp	LABEL(ashr_6_use_ssse3)

 /*
 * The following cases will be handled by ashr_5
 *  rcx(offset of rsi)  rax(offset of rdi)	relative offset  	  corresponding case
 *      n(5~15)		n - 5		5((16 - (n -5) + n)%16	 ashr_5
 *
 * Based on above operation , start from  (%r9 + rsi) to the left of this cache bank, there is no null byte
 */
	.p2align 4
LABEL(ashr_5):
	xor	%ecx, %ecx				/*clear ecx */
#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	.p2align 4
LABEL(ashr_5_use_ssse3):
	movdqa	16(%rsi, %rcx), %xmm3
	pcmpeqb	%xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $5, (%rsi, %rcx), %xmm3
	movdqa	%xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	movdqa  16(%rsi, %rcx), %xmm3
	pcmpeqb %xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $5, (%rsi, %rcx), %xmm3
	movdqa  %xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif
	jmp	LABEL(ashr_5_use_ssse3)

/*
 *
 * The following cases will be handled by ashr_4
 *  rcx(offset of rsi)  rax(offset of rdi)	relative offset  	  corresponding case
 *      n(4~15)		n - 4		4((16 - (n -4) + n)%16	 ashr_4
 *
 * Based on above operation , start from  (%r9 + rsi) to the left of this cache bank, there is no null byte
 */
	.p2align 4
LABEL(ashr_4):
	xor	%ecx, %ecx				/*clear ecx */
#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	.p2align 4
LABEL(ashr_4_use_ssse3):
	movdqa	16(%rsi, %rcx), %xmm3
	pcmpeqb	%xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $4, (%rsi, %rcx), %xmm3
	movdqa	%xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	movdqa  16(%rsi, %rcx), %xmm3
	pcmpeqb %xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $4, (%rsi, %rcx), %xmm3
	movdqa  %xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif
	jmp	LABEL(ashr_4_use_ssse3)

/*
 *
 * The following cases will be handled by ashr_3
 *  rcx(offset of rsi)  rax(offset of rdi)	relative offset  	  corresponding case
 *      n(3~15)		n - 3		3((16 - (n -3) + n)%16	 ashr_3
 *
 * Based on above operation , start from  (%r9 + rsi) to the left of this cache bank, there is no null byte
 */
	.p2align 4
LABEL(ashr_3):
	xor	%ecx, %ecx				/*clear ecx */
#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	.p2align 4
LABEL(ashr_3_use_ssse3):
	movdqa	16(%rsi, %rcx), %xmm3
	pcmpeqb	%xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $3, (%rsi, %rcx), %xmm3
	movdqa	%xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	movdqa  16(%rsi, %rcx), %xmm3
	pcmpeqb %xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $3, (%rsi, %rcx), %xmm3
	movdqa  %xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif
	jmp	LABEL(ashr_3_use_ssse3)

/*
 *
 * The following cases will be handled by ashr_2
 *  rcx(offset of rsi)  rax(offset of rdi)	relative offset  	  corresponding case
 *      n(2~15)		n - 2		2((16 - (n -2) + n)%16	 ashr_2
 *
 * Based on above operation , start from  (%r9 + rsi) to the left of this cache bank, there is no null byte
 */
	.p2align 4
LABEL(ashr_2):
	xor	%ecx, %ecx				/*clear ecx */
#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	.p2align 4
LABEL(ashr_2_use_ssse3):
	movdqa	16(%rsi, %rcx), %xmm3
	pcmpeqb	%xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $2, (%rsi, %rcx), %xmm3
	movdqa	%xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	movdqa  16(%rsi, %rcx), %xmm3
	pcmpeqb %xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $2, (%rsi, %rcx), %xmm3
	movdqa  %xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif
	jmp	LABEL(ashr_2_use_ssse3)

/*
 *
 * The following cases will be handled by ashr_1
 *  rcx(offset of rsi)  rax(offset of rdi)  relative offset  		corresponding case
 *	n(1~15)		n - 1	   	1 ((16 - (n -1) + n)%16	 ashr_1
 *
 * Based on above operation , start from  (%r9 + rsi) to the left of this cache bank, there is no null byte
 */
	.p2align 4
LABEL(ashr_1):
	xor	%ecx, %ecx				/*clear ecx */
#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	.p2align 4
LABEL(ashr_1_use_ssse3):
	movdqa	16(%rsi, %rcx), %xmm3
	pcmpeqb	%xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif

	palignr $1, (%rsi, %rcx), %xmm3
	movdqa	%xmm3, (%rdi, %rcx)
	add	$16, %rcx
#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif

	movdqa  16(%rsi, %rcx), %xmm3
	pcmpeqb %xmm3, %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	jnz	LABEL(unaligned_exit)
#ifdef USE_AS_STRNCPY
	sub	$16, %r8
	jbe	LABEL(strncpy_truncation_unaligned)
#endif
	palignr $1, (%rsi, %rcx), %xmm3
	movdqa  %xmm3, (%rdi, %rcx)
	add	$16, %rcx

#ifdef USE_AS_STRNCPY
	cmp	%r10, %r8
	jbe	LABEL(unaligned_exit)
#endif
	jmp	LABEL(ashr_1_use_ssse3)

	.p2align 4
LABEL(less32bytes):
	xor	%ecx, %ecx
LABEL(unaligned_exit):
	add	%r9, %rsi		/* r9 stores original offset of rsi*/
	mov	%rcx, %r9
	mov	%r10, %rcx
	shl	%cl, %edx		/* after shl, calculate the exact number to be filled*/
	mov	%r9, %rcx
	.p2align 4
LABEL(aligned_exit):
	add	%rcx, %rdi		/*locate exact address for rdi */
LABEL(less16bytes):
	add	%rcx, %rsi		/*locate exact address for rsi */
LABEL(aligned_16bytes):
#ifdef USE_AS_STRNCPY
	mov     $1, %r9d
	lea     -1(%r8), %rcx
	shl     %cl, %r9d
	cmp     $32, %r8
	ja      LABEL(strncpy_tail)
	or      %r9d, %edx
LABEL(strncpy_tail):
#endif
	bsf	%rdx, %rcx		/*If a least significant 1 bit in %rdx is found, its bit index is stored in %rcx*/
	lea	LABEL(tail_table)(%rip), %r11
	movslq	(%r11, %rcx,4), %rcx
	lea	(%r11, %rcx), %rcx
	jmp	*%rcx

#ifdef USE_AS_STRNCPY
	.p2align 4
LABEL(less32bytes_strncpy_truncation):
	xor     %ecx, %ecx
LABEL(strncpy_truncation_unaligned):
	add      %r9, %rsi
LABEL(strncpy_truncation_aligned):
	add      %rcx, %rdi
	add      %rcx, %rsi
	add     $16, %r8
	lea     -1(%r8), %rcx
	lea     LABEL(tail_table)(%rip), %r11
	movslq  (%r11, %rcx,4), %rcx
	lea     (%r11, %rcx), %rcx
	jmp     *%rcx
	.p2align 4
LABEL(strncpy_exitz):
	mov     %rdi, %rax
	ret
#endif

#ifdef USE_AS_STRNCPY
	.p2align 4
LABEL(strncpy_fill_tail):
	mov	%rax, %rdx
	movzx	%cl, %rax
	mov	%r8, %rcx
	add	%rax, %rdi
	xor	%eax, %eax
	shr	$3, %ecx
	jz	LABEL(strncpy_fill_less_8)

	rep	stosq
LABEL(strncpy_fill_less_8):
	mov	%r8, %rcx
	and	$7, %ecx
	jz	LABEL(strncpy_fill_return)
LABEL(strncpy_fill_less_7):
	sub	$1, %ecx
	mov	%al, (%rdi, %rcx)
	jnz	LABEL(strncpy_fill_less_7)
LABEL(strncpy_fill_return):
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rdx)
	sbb	$-1, %rdx
#endif
	mov	%rdx, %rax
	ret
#endif
	.p2align 4
LABEL(tail_0):
	mov	(%rsi), %cl
	mov	%cl, (%rdi)
#ifdef USE_AS_STPCPY
	mov	%rdi, %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$1, %cl
	sub	$1, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret
	.p2align 4
LABEL(tail_1):
	mov	(%rsi), %cx
	mov	%cx, (%rdi)
#ifdef USE_AS_STPCPY
	lea	1(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$2, %cl
	sub	$2, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret
	.p2align 4
LABEL(tail_2):
	mov	(%rsi), %cx
	mov	%cx, (%rdi)
	mov	1(%rsi), %cx
	mov	%cx, 1(%rdi)
#ifdef USE_AS_STPCPY
	lea	2(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$3, %cl
	sub	$3, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret
	.p2align 4
LABEL(tail_3):
	mov	(%rsi), %ecx
	mov	%ecx, (%rdi)
#ifdef USE_AS_STPCPY
	lea	3(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$4, %cl
	sub	$4, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret
	.p2align 4
LABEL(tail_4):
	mov	(%rsi), %ecx
	mov	%ecx, (%rdi)
	mov	1(%rsi), %edx
	mov	%edx, 1(%rdi)
#ifdef USE_AS_STPCPY
	lea	4(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$5, %cl
	sub	$5, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret
	.p2align 4
LABEL(tail_5):
	mov	(%rsi), %ecx
	mov	%ecx, (%rdi)
	mov	2(%rsi), %edx
	mov	%edx, 2(%rdi)
#ifdef USE_AS_STPCPY
	lea	5(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$6, %cl
	sub	$6, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret
	.p2align 4
LABEL(tail_6):
	mov	(%rsi), %ecx
	mov	%ecx, (%rdi)
	mov	3(%rsi), %edx
	mov	%edx,3(%rdi)
#ifdef USE_AS_STPCPY
	lea	6(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$7, %cl
	sub	$7, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret

	.p2align 4
LABEL(tail_7):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
#ifdef USE_AS_STPCPY
	lea	7(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$8, %cl
	sub	$8, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret

	.p2align 4
LABEL(tail_8):

	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	5(%rsi), %edx
	mov	%edx, 5(%rdi)
#ifdef USE_AS_STPCPY
	lea	8(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$9, %cl
	sub	$9, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret

	.p2align 4
LABEL(tail_9):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	6(%rsi), %edx
	mov	%edx, 6(%rdi)
#ifdef USE_AS_STPCPY
	lea	9(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$10, %cl
	sub	$10, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret

	.p2align 4
LABEL(tail_10):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	7(%rsi), %edx
	mov	%edx, 7(%rdi)
#ifdef USE_AS_STPCPY
	lea	10(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$11, %cl
	sub	$11, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret
	.p2align 4
LABEL(tail_11):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	8(%rsi), %edx
	mov	%edx, 8(%rdi)
#ifdef USE_AS_STPCPY
	lea	11(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$12, %cl
	sub	$12, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret
	.p2align 4
LABEL(tail_12):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	5(%rsi), %rcx
	mov	%rcx, 5(%rdi)
#ifdef USE_AS_STPCPY
	lea	12(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$13, %cl
	sub	$13, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret

	.p2align 4
LABEL(tail_13):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	6(%rsi), %rcx
	mov	%rcx, 6(%rdi)
#ifdef USE_AS_STPCPY
	lea	13(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$14, %cl
	sub	$14, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret

	.p2align 4
LABEL(tail_14):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	7(%rsi), %rcx
	mov	%rcx, 7(%rdi)
#ifdef USE_AS_STPCPY
	lea	14(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$15, %cl
	sub	$15, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret

LABEL(tail_15):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	8(%rsi), %rdx
	mov	%rdx, 8(%rdi)
#ifdef USE_AS_STPCPY
	lea	15(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$16, %cl
	sub	$16, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif

	ret

	.p2align 4
LABEL(tail_16):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	8(%rsi), %rdx
	mov	%rdx, 8(%rdi)
	mov	16(%rsi), %cl
	mov	%cl, 16(%rdi)
#ifdef USE_AS_STPCPY
	lea	16(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$17, %cl
	sub	$17, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret
	.p2align 4
LABEL(tail_17):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	8(%rsi), %rdx
	mov	%rdx, 8(%rdi)
	mov	16(%rsi), %cx
	mov	%cx, 16(%rdi)
#ifdef USE_AS_STPCPY
	lea	17(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$18, %cl
	sub	$18, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret

	.p2align 4
LABEL(tail_18):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	8(%rsi), %rdx
	mov	%rdx, 8(%rdi)
	mov	15(%rsi), %ecx
	mov	%ecx,15(%rdi)
#ifdef USE_AS_STPCPY
	lea	18(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$19, %cl
	sub	$19, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret

	.p2align 4
LABEL(tail_19):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	8(%rsi), %rdx
	mov	%rdx, 8(%rdi)
	mov	16(%rsi), %ecx
	mov	%ecx, 16(%rdi)
#ifdef USE_AS_STPCPY
	lea	19(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$20, %cl
	sub	$20, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret
	.p2align 4
LABEL(tail_20):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	8(%rsi), %rdx
	mov	%rdx, 8(%rdi)
	mov	13(%rsi), %rcx
	mov	%rcx, 13(%rdi)
#ifdef USE_AS_STPCPY
	lea	20(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$21, %cl
	sub	$21, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret
	.p2align 4
LABEL(tail_21):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	8(%rsi), %rdx
	mov	%rdx, 8(%rdi)
	mov	14(%rsi), %rcx
	mov	%rcx, 14(%rdi)
#ifdef USE_AS_STPCPY
	lea	21(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$22, %cl
	sub	$22, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret

	.p2align 4
LABEL(tail_22):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	8(%rsi), %rdx
	mov	%rdx, 8(%rdi)
	mov	15(%rsi), %rcx
	mov	%rcx, 15(%rdi)
#ifdef USE_AS_STPCPY
	lea	22(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$23, %cl
	sub	$23, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret

	.p2align 4
LABEL(tail_23):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	8(%rsi), %rdx
	mov	%rdx, 8(%rdi)
	mov	16(%rsi), %rcx
	mov	%rcx, 16(%rdi)
#ifdef USE_AS_STPCPY
	lea	23(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$24, %cl
	sub	$24, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif

	ret

	.p2align 4
LABEL(tail_24):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	8(%rsi), %rdx
	mov	%rdx, 8(%rdi)
	mov	16(%rsi), %rcx
	mov	%rcx, 16(%rdi)
	mov	21(%rsi), %edx
	mov	%edx, 21(%rdi)
#ifdef USE_AS_STPCPY
	lea	24(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$25, %cl
	sub	$25, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret

	.p2align 4
LABEL(tail_25):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	8(%rsi), %rdx
	mov	%rdx, 8(%rdi)
	mov	16(%rsi), %rcx
	mov	%rcx, 16(%rdi)
	mov	22(%rsi), %edx
	mov	%edx, 22(%rdi)
#ifdef USE_AS_STPCPY
	lea	25(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$26, %cl
	sub	$26, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret

	.p2align 4
LABEL(tail_26):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	8(%rsi), %rdx
	mov	%rdx, 8(%rdi)
	mov	16(%rsi), %rcx
	mov	%rcx, 16(%rdi)
	mov	23(%rsi), %edx
	mov	%edx, 23(%rdi)
#ifdef USE_AS_STPCPY
	lea	26(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$27, %cl
	sub	$27, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret

	.p2align 4
LABEL(tail_27):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	8(%rsi), %rdx
	mov	%rdx, 8(%rdi)
	mov	16(%rsi), %rcx
	mov	%rcx, 16(%rdi)
	mov	24(%rsi), %edx
	mov	%edx, 24(%rdi)
#ifdef USE_AS_STPCPY
	lea	27(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$28, %cl
	sub	$28, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret
	.p2align 4
LABEL(tail_28):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	8(%rsi), %rdx
	mov	%rdx, 8(%rdi)
	mov	16(%rsi), %rcx
	mov	%rcx, 16(%rdi)
	mov	21(%rsi), %rdx
	mov	%rdx, 21(%rdi)
#ifdef USE_AS_STPCPY
	lea	28(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$29, %cl
	sub	$29, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif

	ret

	.p2align 4
LABEL(tail_29):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	8(%rsi), %rdx
	mov	%rdx, 8(%rdi)
	mov	16(%rsi), %rcx
	mov	%rcx, 16(%rdi)
	mov	22(%rsi), %rdx
	mov	%rdx, 22(%rdi)
#ifdef USE_AS_STPCPY
	lea	29(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$30, %cl
	sub	$30, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif

	ret


	.p2align 4
LABEL(tail_30):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	8(%rsi), %rdx
	mov	%rdx, 8(%rdi)
	mov	16(%rsi), %rcx
	mov	%rcx, 16(%rdi)
	mov	23(%rsi), %rdx
	mov	%rdx, 23(%rdi)
#ifdef USE_AS_STPCPY
	lea	30(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$31, %cl
	sub	$31, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret

	.p2align 4
LABEL(tail_31):
	mov	(%rsi), %rcx
	mov	%rcx, (%rdi)
	mov	8(%rsi), %rdx
	mov	%rdx, 8(%rdi)
	mov	16(%rsi), %rcx
	mov	%rcx, 16(%rdi)
	mov	24(%rsi), %rdx
	mov	%rdx, 24(%rdi)
#ifdef USE_AS_STPCPY
	lea	31(%rdi), %rax
#endif
#ifdef USE_AS_STRNCPY
	mov	$32, %cl
	sub	$32, %r8
	jnz	LABEL(strncpy_fill_tail)
#ifdef USE_AS_STPCPY
	cmpb	$1, (%rax)
	sbb	$-1, %rax
#endif
#endif
	ret
	cfi_endproc
	.size	STRCPY_SSSE3, .-STRCPY_SSSE3

	.p2align 4
	.section .rodata.ssse3,"a",@progbits
LABEL(tail_table):
	.int	LABEL(tail_0) - LABEL(tail_table)
	.int	LABEL(tail_1) - LABEL(tail_table)
	.int	LABEL(tail_2) - LABEL(tail_table)
	.int	LABEL(tail_3) - LABEL(tail_table)
	.int	LABEL(tail_4) - LABEL(tail_table)
	.int	LABEL(tail_5) - LABEL(tail_table)
	.int	LABEL(tail_6) - LABEL(tail_table)
	.int	LABEL(tail_7) - LABEL(tail_table)
	.int	LABEL(tail_8) - LABEL(tail_table)
	.int	LABEL(tail_9) - LABEL(tail_table)
	.int	LABEL(tail_10) - LABEL(tail_table)
	.int	LABEL(tail_11) - LABEL(tail_table)
	.int	LABEL(tail_12) - LABEL(tail_table)
	.int	LABEL(tail_13) - LABEL(tail_table)
	.int	LABEL(tail_14) - LABEL(tail_table)
	.int	LABEL(tail_15) - LABEL(tail_table)
	.int	LABEL(tail_16) - LABEL(tail_table)
	.int	LABEL(tail_17) - LABEL(tail_table)
	.int	LABEL(tail_18) - LABEL(tail_table)
	.int	LABEL(tail_19) - LABEL(tail_table)
	.int	LABEL(tail_20) - LABEL(tail_table)
	.int	LABEL(tail_21) - LABEL(tail_table)
	.int	LABEL(tail_22) - LABEL(tail_table)
	.int	LABEL(tail_23) - LABEL(tail_table)
	.int	LABEL(tail_24) - LABEL(tail_table)
	.int	LABEL(tail_25) - LABEL(tail_table)
	.int	LABEL(tail_26) - LABEL(tail_table)
	.int	LABEL(tail_27) - LABEL(tail_table)
	.int	LABEL(tail_28) - LABEL(tail_table)
	.int	LABEL(tail_29) - LABEL(tail_table)
	.int	LABEL(tail_30) - LABEL(tail_table)
	.int	LABEL(tail_31) - LABEL(tail_table)

	.p2align 4
LABEL(unaligned_table):
	.int	LABEL(ashr_0) - LABEL(unaligned_table)
	.int	LABEL(ashr_1) - LABEL(unaligned_table)
	.int	LABEL(ashr_2) - LABEL(unaligned_table)
	.int	LABEL(ashr_3) - LABEL(unaligned_table)
	.int	LABEL(ashr_4) - LABEL(unaligned_table)
	.int	LABEL(ashr_5) - LABEL(unaligned_table)
	.int	LABEL(ashr_6) - LABEL(unaligned_table)
	.int	LABEL(ashr_7) - LABEL(unaligned_table)
	.int	LABEL(ashr_8) - LABEL(unaligned_table)
	.int	LABEL(ashr_9) - LABEL(unaligned_table)
	.int	LABEL(ashr_10) - LABEL(unaligned_table)
	.int	LABEL(ashr_11) - LABEL(unaligned_table)
	.int	LABEL(ashr_12) - LABEL(unaligned_table)
	.int	LABEL(ashr_13) - LABEL(unaligned_table)
	.int	LABEL(ashr_14) - LABEL(unaligned_table)
	.int	LABEL(ashr_15) - LABEL(unaligned_table)

# undef ENTRY
# define ENTRY(name) \
	.type STRCPY_SSE2, @function; \
	.align 16; \
	STRCPY_SSE2: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size STRCPY_SSE2, .-STRCPY_SSE2
# undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal strcpy calls through a PLT.
   The speedup we get from using SSSE3 instruction is likely eaten away
   by the indirect call in the PLT.  */
# define libc_hidden_builtin_def(name) \
	.globl __GI_STRCPY; __GI_STRCPY = STRCPY_SSE2
# undef libc_hidden_def
# define libc_hidden_def(name) \
	.globl __GI___STRCPY; __GI___STRCPY = STRCPY_SSE2
#endif

#ifndef USE_AS_STRNCPY
#include "../strcpy.S"
#endif
