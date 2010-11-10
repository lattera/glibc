/* strcmp with SSE4.2
   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
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

#ifdef USE_AS_STRNCMP
/* Since the counter, %r11, is unsigned, we branch to strcmp_exitz
   if the new counter > the old one or is 0.  */
# define UPDATE_STRNCMP_COUNTER				\
	/* calculate left number to compare */		\
	lea	-16(%rcx, %r11), %r9;			\
	cmp	%r9, %r11;				\
	jb	LABEL(strcmp_exitz_sse4_2);		\
	test	%r9, %r9;				\
	je	LABEL(strcmp_exitz_sse4_2);		\
	mov	%r9, %r11

# define STRCMP_SSE42	__strncmp_sse42
# define STRCMP_SSSE3	__strncmp_ssse3
# define STRCMP_SSE2	__strncmp_sse2
# define __GI_STRCMP	__GI_strncmp
#elif defined USE_AS_STRCASECMP_L
# include "locale-defines.h"

# define UPDATE_STRNCMP_COUNTER

# define STRCMP_SSE42	__strcasecmp_l_sse42
# define STRCMP_SSSE3	__strcasecmp_l_ssse3
# define STRCMP_SSE2	__strcasecmp_l_sse2
# define __GI_STRCMP	__GI___strcasecmp_l
#elif defined USE_AS_STRNCASECMP_L
# include "locale-defines.h"

/* Since the counter, %r11, is unsigned, we branch to strcmp_exitz
   if the new counter > the old one or is 0.  */
# define UPDATE_STRNCMP_COUNTER				\
	/* calculate left number to compare */		\
	lea	-16(%rcx, %r11), %r9;			\
	cmp	%r9, %r11;				\
	jb	LABEL(strcmp_exitz_sse4_2);		\
	test	%r9, %r9;				\
	je	LABEL(strcmp_exitz_sse4_2);		\
	mov	%r9, %r11

# define STRCMP_SSE42	__strncasecmp_l_sse42
# define STRCMP_SSSE3	__strncasecmp_l_ssse3
# define STRCMP_SSE2	__strncasecmp_l_sse2
# define __GI_STRCMP	__GI___strncasecmp_l
#else
# define UPDATE_STRNCMP_COUNTER
# ifndef STRCMP
#  define STRCMP	strcmp
#  define STRCMP_SSE42	__strcmp_sse42
#  define STRCMP_SSSE3	__strcmp_ssse3
#  define STRCMP_SSE2	__strcmp_sse2
#  define __GI_STRCMP	__GI_strcmp
# endif
#endif

#ifndef LABEL
# define LABEL(l) L(l)
#endif

/* Define multiple versions only for the definition in libc.  Don't
   define multiple versions for strncmp in static library since we
   need strncmp before the initialization happened.  */
#if (defined SHARED || !defined USE_AS_STRNCMP) && !defined NOT_IN_libc
	.text
ENTRY(STRCMP)
	.type	STRCMP, @gnu_indirect_function
	cmpl	$0, __cpu_features+KIND_OFFSET(%rip)
	jne	1f
	call	__init_cpu_features
1:
	leaq	STRCMP_SSE42(%rip), %rax
	testl	$bit_SSE4_2, __cpu_features+CPUID_OFFSET+index_SSE4_2(%rip)
	jnz	2f
	leaq	STRCMP_SSSE3(%rip), %rax
	testl	$bit_SSSE3, __cpu_features+CPUID_OFFSET+index_SSSE3(%rip)
	jnz	2f
	leaq	STRCMP_SSE2(%rip), %rax
2:	ret
END(STRCMP)

# ifdef USE_AS_STRCASECMP_L
ENTRY(__strcasecmp)
	.type	__strcasecmp, @gnu_indirect_function
	cmpl	$0, __cpu_features+KIND_OFFSET(%rip)
	jne	1f
	call	__init_cpu_features
1:
	leaq	__strcasecmp_sse42(%rip), %rax
	testl	$bit_SSE4_2, __cpu_features+CPUID_OFFSET+index_SSE4_2(%rip)
	jnz	2f
	leaq	__strcasecmp_ssse3(%rip), %rax
	testl	$bit_SSSE3, __cpu_features+CPUID_OFFSET+index_SSSE3(%rip)
	jnz	2f
	leaq	__strcasecmp_sse2(%rip), %rax
2:	ret
END(__strcasecmp)
weak_alias (__strcasecmp, strcasecmp)
# endif
# ifdef USE_AS_STRNCASECMP_L
ENTRY(__strncasecmp)
	.type	__strncasecmp, @gnu_indirect_function
	cmpl	$0, __cpu_features+KIND_OFFSET(%rip)
	jne	1f
	call	__init_cpu_features
1:
	leaq	__strncasecmp_sse42(%rip), %rax
	testl	$bit_SSE4_2, __cpu_features+CPUID_OFFSET+index_SSE4_2(%rip)
	jnz	2f
	leaq	__strncasecmp_ssse3(%rip), %rax
	testl	$bit_SSSE3, __cpu_features+CPUID_OFFSET+index_SSSE3(%rip)
	jnz	2f
	leaq	__strncasecmp_sse2(%rip), %rax
2:	ret
END(__strncasecmp)
weak_alias (__strncasecmp, strncasecmp)
# endif

/* We use 0x1a:
	_SIDD_SBYTE_OPS
	| _SIDD_CMP_EQUAL_EACH
	| _SIDD_NEGATIVE_POLARITY
	| _SIDD_LEAST_SIGNIFICANT
   on pcmpistri to find out if two 16byte data elements are the same
   and the offset of the first different byte.  There are 4 cases:

   1. Both 16byte data elements are valid and identical.
   2. Both 16byte data elements have EOS and identical.
   3. Both 16byte data elements are valid and they differ at offset X.
   4. At least one 16byte data element has EOS at offset X.  Two 16byte
      data elements must differ at or before offset X.

   Here is the table of ECX, CFlag, ZFlag and SFlag for 4 cases:

   case		ECX	CFlag	ZFlag	SFlag
    1		16	  0	  0	  0
    2		16	  0	  1	  1
    3		 X	  1	  0	  0
    4	       0 <= X	  1	 0/1	 0/1

   We exit from the loop for cases 2, 3 and 4 with jbe which branches
   when either CFlag or ZFlag is 1.  If CFlag == 0, we return 0 for
   case 2.  */

	/* Put all SSE 4.2 functions together.  */
	.section .text.sse4.2,"ax",@progbits
	.align	16
	.type	STRCMP_SSE42, @function
# ifdef USE_AS_STRCASECMP_L
ENTRY (__strcasecmp_sse42)
	movq	__libc_tsd_LOCALE@gottpoff(%rip),%rax
	movq	%fs:(%rax),%rdx

	// XXX 5 byte should be before the function
	/* 5-byte NOP.  */
	.byte	0x0f,0x1f,0x44,0x00,0x00
END (__strcasecmp_sse42)
	/* FALLTHROUGH to strcasecmp_l.  */
# endif
# ifdef USE_AS_STRNCASECMP_L
ENTRY (__strncasecmp_sse42)
	movq	__libc_tsd_LOCALE@gottpoff(%rip),%rax
	movq	%fs:(%rax),%rcx

	// XXX 5 byte should be before the function
	/* 5-byte NOP.  */
	.byte	0x0f,0x1f,0x44,0x00,0x00
END (__strncasecmp_sse42)
	/* FALLTHROUGH to strncasecmp_l.  */
# endif

STRCMP_SSE42:
	cfi_startproc
	CALL_MCOUNT

/*
 * This implementation uses SSE to compare up to 16 bytes at a time.
 */
# ifdef USE_AS_STRCASECMP_L
	/* We have to fall back on the C implementation for locales
	   with encodings not matching ASCII for single bytes.  */
#  if LOCALE_T___LOCALES != 0 || LC_CTYPE != 0
	movq	LOCALE_T___LOCALES+LC_CTYPE*8(%rdx), %rax
#  else
	movq	(%rdx), %rax
#  endif
	testl	$0, LOCALE_DATA_VALUES+_NL_CTYPE_NONASCII_CASE*SIZEOF_VALUES(%rax)
	jne	__strcasecmp_l_nonascii
# endif
# ifdef USE_AS_STRNCASECMP_L
	/* We have to fall back on the C implementation for locales
	   with encodings not matching ASCII for single bytes.  */
#  if LOCALE_T___LOCALES != 0 || LC_CTYPE != 0
	movq	LOCALE_T___LOCALES+LC_CTYPE*8(%rcx), %rax
#  else
	movq	(%rcx), %rax
#  endif
	testl	$0, LOCALE_DATA_VALUES+_NL_CTYPE_NONASCII_CASE*SIZEOF_VALUES(%rax)
	jne	__strncasecmp_l_nonascii
# endif

# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	test	%rdx, %rdx
	je	LABEL(strcmp_exitz_sse4_2)
	cmp	$1, %rdx
	je	LABEL(Byte0_sse4_2)
	mov	%rdx, %r11
# endif
	mov	%esi, %ecx
	mov	%edi, %eax
/* Use 64bit AND here to avoid long NOP padding.  */
	and	$0x3f, %rcx		/* rsi alignment in cache line */
	and	$0x3f, %rax		/* rdi alignment in cache line */
# if defined USE_AS_STRCASECMP_L || defined USE_AS_STRNCASECMP_L
	.section .rodata.cst16,"aM",@progbits,16
	.align 16
.Lbelowupper_sse4:
	.quad	0x4040404040404040
	.quad	0x4040404040404040
.Ltopupper_sse4:
	.quad	0x5b5b5b5b5b5b5b5b
	.quad	0x5b5b5b5b5b5b5b5b
.Ltouppermask_sse4:
	.quad	0x2020202020202020
	.quad	0x2020202020202020
	.previous
	movdqa	.Lbelowupper_sse4(%rip), %xmm4
#  define UCLOW_reg %xmm4
	movdqa	.Ltopupper_sse4(%rip), %xmm5
#  define UCHIGH_reg %xmm5
	movdqa	.Ltouppermask_sse4(%rip), %xmm6
#  define LCQWORD_reg %xmm6
# endif
	cmp	$0x30, %ecx
	ja	LABEL(crosscache_sse4_2)/* rsi: 16-byte load will cross cache line */
	cmp	$0x30, %eax
	ja	LABEL(crosscache_sse4_2)/* rdi: 16-byte load will cross cache line */
	movdqu	(%rdi), %xmm1
	movdqu	(%rsi), %xmm2
# if defined USE_AS_STRCASECMP_L || defined USE_AS_STRNCASECMP_L
#  define TOLOWER(reg1, reg2) \
	movdqa	reg1, %xmm7;					\
	movdqa	UCHIGH_reg, %xmm8;				\
	movdqa	reg2, %xmm9;					\
	movdqa	UCHIGH_reg, %xmm10;				\
	pcmpgtb	UCLOW_reg, %xmm7;				\
	pcmpgtb	reg1, %xmm8;					\
	pcmpgtb	UCLOW_reg, %xmm9;				\
	pcmpgtb	reg2, %xmm10;					\
	pand	%xmm8, %xmm7;					\
	pand	%xmm10, %xmm9;					\
	pand	LCQWORD_reg, %xmm7;				\
	pand	LCQWORD_reg, %xmm9;				\
	por	%xmm7, reg1;					\
	por	%xmm9, reg2
	TOLOWER (%xmm1, %xmm2)
# else
#  define TOLOWER(reg1, reg2)
# endif
	pxor	%xmm0, %xmm0		/* clear %xmm0 for null char checks */
	pcmpeqb	%xmm1, %xmm0		/* Any null chars? */
	pcmpeqb	%xmm2, %xmm1		/* compare first 16 bytes for equality */
	psubb	%xmm0, %xmm1		/* packed sub of comparison results*/
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx		/* if first 16 bytes are same, edx == 0xffff */
	jnz	LABEL(less16bytes_sse4_2)/* If not, find different value or null char */
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)/* finish comparision */
# endif
	add	$16, %rsi		/* prepare to search next 16 bytes */
	add	$16, %rdi		/* prepare to search next 16 bytes */

	/*
	 * Determine source and destination string offsets from 16-byte alignment.
	 * Use relative offset difference between the two to determine which case
	 * below to use.
	 */
	.p2align 4
LABEL(crosscache_sse4_2):
	and	$0xfffffffffffffff0, %rsi	/* force %rsi is 16 byte aligned */
	and	$0xfffffffffffffff0, %rdi	/* force %rdi is 16 byte aligned */
	mov	$0xffff, %edx			/* for equivalent offset */
	xor	%r8d, %r8d
	and	$0xf, %ecx			/* offset of rsi */
	and	$0xf, %eax			/* offset of rdi */
	cmp	%eax, %ecx
	je	LABEL(ashr_0_sse4_2)		/* rsi and rdi relative offset same */
	ja	LABEL(bigger_sse4_2)
	mov	%edx, %r8d			/* r8d is offset flag for exit tail */
	xchg	%ecx, %eax
	xchg	%rsi, %rdi
LABEL(bigger_sse4_2):
	lea	15(%rax), %r9
	sub	%rcx, %r9
	lea	LABEL(unaligned_table_sse4_2)(%rip), %r10
	movslq	(%r10, %r9,4), %r9
	lea	(%r10, %r9), %r10
	jmp	*%r10				/* jump to corresponding case */

/*
 * The following cases will be handled by ashr_0
 *  rcx(offset of rsi)  rax(offset of rdi)  relative offset  corresponding case
 *        n(0~15)            n(0~15)           15(15+ n-n)         ashr_0
 */
	.p2align 4
LABEL(ashr_0_sse4_2):

	movdqa	(%rsi), %xmm1
	pxor	%xmm0, %xmm0			/* clear %xmm0 for null char check */
	pcmpeqb	%xmm1, %xmm0			/* Any null chars? */
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpeqb	(%rdi), %xmm1			/* compare 16 bytes for equality */
# else
	movdqa	(%rdi), %xmm2
	TOLOWER (%xmm1, %xmm2)
	pcmpeqb	%xmm2, %xmm1			/* compare 16 bytes for equality */
# endif
	psubb	%xmm0, %xmm1			/* packed sub of comparison results*/
	pmovmskb %xmm1, %r9d
	shr	%cl, %edx			/* adjust 0xffff for offset */
	shr	%cl, %r9d			/* adjust for 16-byte offset */
	sub	%r9d, %edx
	/*
	 * edx must be the same with r9d if in left byte (16-rcx) is equal to
	 * the start from (16-rax) and no null char was seen.
	 */
	jne	LABEL(less32bytes_sse4_2)	/* mismatch or null char */
	UPDATE_STRNCMP_COUNTER
	mov	$16, %rcx
	mov	$16, %r9
	pxor	%xmm0, %xmm0			/* clear xmm0, may have changed above */

	/*
	 * Now both strings are aligned at 16-byte boundary. Loop over strings
	 * checking 32-bytes per iteration.
	 */
	mov	%rcx, %rdx	/* only for offset of sse4 instruction loop*/
	.p2align 4
LABEL(ashr_0_use_sse4_2):
	movdqa	(%rdi,%rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri      $0x1a,(%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	lea	16(%rdx), %rdx
	jbe	LABEL(ashr_0_use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif

	movdqa	(%rdi,%rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri      $0x1a,(%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	lea	16(%rdx), %rdx
	jbe	LABEL(ashr_0_use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif
	jmp	LABEL(ashr_0_use_sse4_2)


	.p2align 4
LABEL(ashr_0_use_sse4_2_exit):
	jnc	LABEL(strcmp_exitz_sse4_2)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	%rcx, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif
	lea	-16(%rdx, %rcx), %rcx
	movzbl	(%rdi, %rcx), %eax
	movzbl	(%rsi, %rcx), %edx
# if defined USE_AS_STRCASECMP_L || defined USE_AS_STRNCASECMP_L
	leaq	_nl_C_LC_CTYPE_tolower+128*4(%rip), %rcx
	movl	(%rcx,%rax,4), %eax
	movl	(%rcx,%rdx,4), %edx
# endif
	sub	%edx, %eax
	ret



/*
 * The following cases will be handled by ashr_1
 * rcx(offset of rsi)  rax(offset of rdi)   relative offset	corresponding case
 *        n(15)            n -15            0(15 +(n-15) - n)         ashr_1
 */
	.p2align 4
LABEL(ashr_1_sse4_2):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0		/* Any null chars? */
	pslldq	$15, %xmm2		/* shift first string to align with second */
	TOLOWER (%xmm1, %xmm2)
	pcmpeqb	%xmm1, %xmm2		/* compare 16 bytes for equality */
	psubb	%xmm0, %xmm2		/* packed sub of comparison results*/
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx		/* adjust 0xffff for offset */
	shr	%cl, %r9d		/* adjust for 16-byte offset */
	sub	%r9d, %edx
	jnz	LABEL(less32bytes_sse4_2)/* mismatch or null char seen */
	movdqa	(%rdi), %xmm3
	UPDATE_STRNCMP_COUNTER

	pxor	%xmm0, %xmm0
	mov	$16, %rcx		/* index for loads*/
	mov	$1, %r9d		/* byte position left over from less32bytes case */
	/*
	 * Setup %r10 value allows us to detect crossing a page boundary.
	 * When %r10 goes positive we have crossed a page boundary and
	 * need to do a nibble.
	 */
	lea	1(%rdi), %r10
	and	$0xfff, %r10		/* offset into 4K page */
	sub	$0x1000, %r10		/* subtract 4K pagesize */
	mov	%rcx, %rdx	/* only for offset of sse4 instruction loop*/

	.p2align 4
LABEL(loop_ashr_1_use_sse4_2):
	add	$16, %r10
	jg	LABEL(nibble_ashr_1_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $1, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri	$0x1a,(%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif

	add	$16, %rdx
	add	$16, %r10
	jg	LABEL(nibble_ashr_1_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $1, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri	$0x1a,(%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif
	add	$16, %rdx
	jmp	LABEL(loop_ashr_1_use_sse4_2)

	.p2align 4
LABEL(nibble_ashr_1_use_sse4_2):
	sub	$0x1000, %r10
	movdqa	-16(%rdi, %rdx), %xmm0
	psrldq	$1, %xmm0
	pcmpistri      $0x3a,%xmm0, %xmm0
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	cmp	%r11, %rcx
	jae	LABEL(nibble_ashr_use_sse4_2_exit)
# endif
	cmp	$14, %ecx
	ja	LABEL(loop_ashr_1_use_sse4_2)

	jmp	LABEL(nibble_ashr_use_sse4_2_exit)

/*
 * The following cases will be handled by ashr_2
 * rcx(offset of rsi)  rax(offset of rdi)   relative offset	corresponding case
 *        n(14~15)            n -14         1(15 +(n-14) - n)         ashr_2
 */
	.p2align 4
LABEL(ashr_2_sse4_2):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$14, %xmm2
	TOLOWER (%xmm1, %xmm2)
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes_sse4_2)
	movdqa	(%rdi), %xmm3
	UPDATE_STRNCMP_COUNTER

	pxor	%xmm0, %xmm0
	mov	$16, %rcx	/* index for loads */
	mov	$2, %r9d	/* byte position left over from less32bytes case */
	/*
	 * Setup %r10 value allows us to detect crossing a page boundary.
	 * When %r10 goes positive we have crossed a page boundary and
	 * need to do a nibble.
	 */
	lea	2(%rdi), %r10
	and	$0xfff, %r10	/* offset into 4K page */
	sub	$0x1000, %r10	/* subtract 4K pagesize */
	mov	%rcx, %rdx	/* only for offset of sse4 instruction loop*/

	.p2align 4
LABEL(loop_ashr_2_use_sse4_2):
	add	$16, %r10
	jg	LABEL(nibble_ashr_2_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $2, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri	$0x1a,(%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif

	add	$16, %rdx
	add	$16, %r10
	jg	LABEL(nibble_ashr_2_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $2, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri	$0x1a,(%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif
	add	$16, %rdx
	jmp	LABEL(loop_ashr_2_use_sse4_2)

	.p2align 4
LABEL(nibble_ashr_2_use_sse4_2):
	sub	$0x1000, %r10
	movdqa	-16(%rdi, %rdx), %xmm0
	psrldq	$2, %xmm0
	pcmpistri      $0x3a,%xmm0, %xmm0
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	cmp	%r11, %rcx
	jae	LABEL(nibble_ashr_use_sse4_2_exit)
# endif
	cmp	$13, %ecx
	ja	LABEL(loop_ashr_2_use_sse4_2)

	jmp	LABEL(nibble_ashr_use_sse4_2_exit)

/*
 * The following cases will be handled by ashr_3
 *  rcx(offset of rsi)  rax(offset of rdi)  relative offset	 corresponding case
 *        n(13~15)            n -13         2(15 +(n-13) - n)         ashr_3
 */
	.p2align 4
LABEL(ashr_3_sse4_2):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$13, %xmm2
	TOLOWER (%xmm1, %xmm2)
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes_sse4_2)
	movdqa	(%rdi), %xmm3

	UPDATE_STRNCMP_COUNTER

	pxor	%xmm0, %xmm0
	mov	$16, %rcx	/* index for loads */
	mov	$3, %r9d	/* byte position left over from less32bytes case */
	/*
	 * Setup %r10 value allows us to detect crossing a page boundary.
	 * When %r10 goes positive we have crossed a page boundary and
	 * need to do a nibble.
	 */
	lea	3(%rdi), %r10
	and	$0xfff, %r10	/* offset into 4K page */
	sub	$0x1000, %r10	/* subtract 4K pagesize */
	mov	%rcx, %rdx	/* only for offset of sse4 instruction loop*/

LABEL(loop_ashr_3_use_sse4_2):
	add	$16, %r10
	jg	LABEL(nibble_ashr_3_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $3, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri	$0x1a,(%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif

	add	$16, %rdx
	add	$16, %r10
	jg	LABEL(nibble_ashr_3_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $3, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri	$0x1a,(%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif
	add	$16, %rdx
	jmp	LABEL(loop_ashr_3_use_sse4_2)

	.p2align 4
LABEL(nibble_ashr_3_use_sse4_2):
	sub	$0x1000, %r10
	movdqa	-16(%rdi, %rdx), %xmm0
	psrldq	$3, %xmm0
	pcmpistri      $0x3a,%xmm0, %xmm0
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	cmp	%r11, %rcx
	jae	LABEL(nibble_ashr_use_sse4_2_exit)
# endif
	cmp	$12, %ecx
	ja	LABEL(loop_ashr_3_use_sse4_2)

	jmp	LABEL(nibble_ashr_use_sse4_2_exit)

/*
 * The following cases will be handled by ashr_4
 *  rcx(offset of rsi)  rax(offset of rdi)  relative offset	 corresponding case
 *        n(12~15)            n -12         3(15 +(n-12) - n)         ashr_4
 */
	.p2align 4
LABEL(ashr_4_sse4_2):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$12, %xmm2
	TOLOWER (%xmm1, %xmm2)
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes_sse4_2)
	movdqa	(%rdi), %xmm3

	UPDATE_STRNCMP_COUNTER

	pxor	%xmm0, %xmm0
	mov	$16, %rcx	/* index for loads */
	mov	$4, %r9d	/* byte position left over from less32bytes case */
	/*
	 * Setup %r10 value allows us to detect crossing a page boundary.
	 * When %r10 goes positive we have crossed a page boundary and
	 * need to do a nibble.
	 */
	lea	4(%rdi), %r10
	and	$0xfff, %r10	/* offset into 4K page */
	sub	$0x1000, %r10	/* subtract 4K pagesize */
	mov	%rcx, %rdx	/* only for offset of sse4 instruction loop*/

	.p2align 4
LABEL(loop_ashr_4_use_sse4_2):
	add	$16, %r10
	jg	LABEL(nibble_ashr_4_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $4, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri	$0x1a,(%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif

	add	$16, %rdx
	add	$16, %r10
	jg	LABEL(nibble_ashr_4_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $4, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri	$0x1a,(%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif
	add	$16, %rdx
	jmp	LABEL(loop_ashr_4_use_sse4_2)

	.p2align 4
LABEL(nibble_ashr_4_use_sse4_2):
	sub	$0x1000, %r10
	movdqa	-16(%rdi, %rdx), %xmm0
	psrldq	$4, %xmm0
	pcmpistri      $0x3a,%xmm0, %xmm0
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	cmp	%r11, %rcx
	jae	LABEL(nibble_ashr_use_sse4_2_exit)
# endif
	cmp	$11, %ecx
	ja	LABEL(loop_ashr_4_use_sse4_2)

	jmp	LABEL(nibble_ashr_use_sse4_2_exit)

/*
 * The following cases will be handled by ashr_5
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset      corresponding case
 *        n(11~15)          n - 11		  4(15 +(n-11) - n)         ashr_5
 */
	.p2align 4
LABEL(ashr_5_sse4_2):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$11, %xmm2
	TOLOWER (%xmm1, %xmm2)
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes_sse4_2)
	movdqa	(%rdi), %xmm3

	UPDATE_STRNCMP_COUNTER

	pxor	%xmm0, %xmm0
	mov	$16, %rcx	/* index for loads */
	mov	$5, %r9d	/* byte position left over from less32bytes case */
	/*
	 * Setup %r10 value allows us to detect crossing a page boundary.
	 * When %r10 goes positive we have crossed a page boundary and
	 * need to do a nibble.
	 */
	lea	5(%rdi), %r10
	and	$0xfff, %r10	/* offset into 4K page */
	sub	$0x1000, %r10	/* subtract 4K pagesize */
	mov	%rcx, %rdx	/* only for offset of sse4 instruction loop*/

	.p2align 4
LABEL(loop_ashr_5_use_sse4_2):
	add	$16, %r10
	jg	LABEL(nibble_ashr_5_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $5, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri	$0x1a,(%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif

	add	$16, %rdx
	add	$16, %r10
	jg	LABEL(nibble_ashr_5_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0

	palignr $5, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri	$0x1a,(%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif
	add	$16, %rdx
	jmp	LABEL(loop_ashr_5_use_sse4_2)

	.p2align 4
LABEL(nibble_ashr_5_use_sse4_2):
	sub	$0x1000, %r10
	movdqa	-16(%rdi, %rdx), %xmm0
	psrldq	$5, %xmm0
	pcmpistri      $0x3a,%xmm0, %xmm0
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	cmp	%r11, %rcx
	jae	LABEL(nibble_ashr_use_sse4_2_exit)
# endif
	cmp	$10, %ecx
	ja	LABEL(loop_ashr_5_use_sse4_2)

	jmp	LABEL(nibble_ashr_use_sse4_2_exit)

/*
 * The following cases will be handled by ashr_6
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset      corresponding case
 *        n(10~15)          n - 10		  5(15 +(n-10) - n)         ashr_6
 */
	.p2align 4
LABEL(ashr_6_sse4_2):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$10, %xmm2
	TOLOWER (%xmm1, %xmm2)
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes_sse4_2)
	movdqa	(%rdi), %xmm3

	UPDATE_STRNCMP_COUNTER

	pxor	%xmm0, %xmm0
	mov	$16, %rcx	/* index for loads */
	mov	$6, %r9d	/* byte position left over from less32bytes case */
	/*
	 * Setup %r10 value allows us to detect crossing a page boundary.
	 * When %r10 goes positive we have crossed a page boundary and
	 * need to do a nibble.
	 */
	lea	6(%rdi), %r10
	and	$0xfff, %r10	/* offset into 4K page */
	sub	$0x1000, %r10	/* subtract 4K pagesize */
	mov	%rcx, %rdx	/* only for offset of sse4 instruction loop*/

	.p2align 4
LABEL(loop_ashr_6_use_sse4_2):
	add	$16, %r10
	jg	LABEL(nibble_ashr_6_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $6, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri $0x1a,(%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif

	add	$16, %rdx
	add	$16, %r10
	jg	LABEL(nibble_ashr_6_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $6, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri $0x1a,(%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif
	add	$16, %rdx
	jmp	LABEL(loop_ashr_6_use_sse4_2)

	.p2align 4
LABEL(nibble_ashr_6_use_sse4_2):
	sub	$0x1000, %r10
	movdqa	-16(%rdi, %rdx), %xmm0
	psrldq	$6, %xmm0
	pcmpistri      $0x3a,%xmm0, %xmm0
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	cmp	%r11, %rcx
	jae	LABEL(nibble_ashr_use_sse4_2_exit)
# endif
	cmp	$9, %ecx
	ja	LABEL(loop_ashr_6_use_sse4_2)

	jmp	LABEL(nibble_ashr_use_sse4_2_exit)

/*
 * The following cases will be handled by ashr_7
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset      corresponding case
 *        n(9~15)          n - 9		  6(15 +(n - 9) - n)         ashr_7
 */
	.p2align 4
LABEL(ashr_7_sse4_2):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$9, %xmm2
	TOLOWER (%xmm1, %xmm2)
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes_sse4_2)
	movdqa	(%rdi), %xmm3

	UPDATE_STRNCMP_COUNTER

	pxor	%xmm0, %xmm0
	mov	$16, %rcx	/* index for loads */
	mov	$7, %r9d	/* byte position left over from less32bytes case */
	/*
	 * Setup %r10 value allows us to detect crossing a page boundary.
	 * When %r10 goes positive we have crossed a page boundary and
	 * need to do a nibble.
	 */
	lea	7(%rdi), %r10
	and	$0xfff, %r10	/* offset into 4K page */
	sub	$0x1000, %r10	/* subtract 4K pagesize */
	mov	%rcx, %rdx	/* only for offset of sse4 instruction loop*/

	.p2align 4
LABEL(loop_ashr_7_use_sse4_2):
	add	$16, %r10
	jg	LABEL(nibble_ashr_7_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $7, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri	$0x1a,(%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif

	add	$16, %rdx
	add	$16, %r10
	jg	LABEL(nibble_ashr_7_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $7, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri	$0x1a,(%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif
	add	$16, %rdx
	jmp	LABEL(loop_ashr_7_use_sse4_2)

	.p2align 4
LABEL(nibble_ashr_7_use_sse4_2):
	sub	$0x1000, %r10
	movdqa	-16(%rdi, %rdx), %xmm0
	psrldq	$7, %xmm0
	pcmpistri      $0x3a,%xmm0, %xmm0
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	cmp	%r11, %rcx
	jae	LABEL(nibble_ashr_use_sse4_2_exit)
# endif
	cmp	$8, %ecx
	ja	LABEL(loop_ashr_7_use_sse4_2)

	jmp	LABEL(nibble_ashr_use_sse4_2_exit)

/*
 *  The following cases will be handled by ashr_8
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset	 corresponding case
 *        n(8~15)          n - 8		  7(15 +(n - 8) - n)         ashr_8
 */
	.p2align 4
LABEL(ashr_8_sse4_2):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$8, %xmm2
	TOLOWER (%xmm1, %xmm2)
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes_sse4_2)
	movdqa	(%rdi), %xmm3

	UPDATE_STRNCMP_COUNTER

	pxor	%xmm0, %xmm0
	mov	$16, %rcx	/* index for loads */
	mov	$8, %r9d	/* byte position left over from less32bytes case */
	/*
	 * Setup %r10 value allows us to detect crossing a page boundary.
	 * When %r10 goes positive we have crossed a page boundary and
	 * need to do a nibble.
	 */
	lea	8(%rdi), %r10
	and	$0xfff, %r10	/* offset into 4K page */
	sub	$0x1000, %r10	/* subtract 4K pagesize */
	mov	%rcx, %rdx	/* only for offset of sse4 instruction loop*/

	.p2align 4
LABEL(loop_ashr_8_use_sse4_2):
	add	$16, %r10
	jg	LABEL(nibble_ashr_8_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $8, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri $0x1a, (%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif

	add	$16, %rdx
	add	$16, %r10
	jg	LABEL(nibble_ashr_8_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $8, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri $0x1a, (%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif
	add	$16, %rdx
	jmp	LABEL(loop_ashr_8_use_sse4_2)

	.p2align 4
LABEL(nibble_ashr_8_use_sse4_2):
	sub	$0x1000, %r10
	movdqa	-16(%rdi, %rdx), %xmm0
	psrldq	$8, %xmm0
	pcmpistri      $0x3a,%xmm0, %xmm0
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	cmp	%r11, %rcx
	jae	LABEL(nibble_ashr_use_sse4_2_exit)
# endif
	cmp	$7, %ecx
	ja	LABEL(loop_ashr_8_use_sse4_2)

	jmp	LABEL(nibble_ashr_use_sse4_2_exit)

/*
 *  The following cases will be handled by ashr_9
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset	 corresponding case
 *        n(7~15)          n - 7		  8(15 +(n - 7) - n)         ashr_9
 */
	.p2align 4
LABEL(ashr_9_sse4_2):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$7, %xmm2
	TOLOWER (%xmm1, %xmm2)
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes_sse4_2)
	movdqa	(%rdi), %xmm3

	UPDATE_STRNCMP_COUNTER

	pxor	%xmm0, %xmm0
	mov	$16, %rcx	/* index for loads */
	mov	$9, %r9d	/* byte position left over from less32bytes case */
	/*
	 * Setup %r10 value allows us to detect crossing a page boundary.
	 * When %r10 goes positive we have crossed a page boundary and
	 * need to do a nibble.
	 */
	lea	9(%rdi), %r10
	and	$0xfff, %r10	/* offset into 4K page */
	sub	$0x1000, %r10	/* subtract 4K pagesize */
	mov	%rcx, %rdx	/* only for offset of sse4 instruction loop*/

	.p2align 4
LABEL(loop_ashr_9_use_sse4_2):
	add	$16, %r10
	jg	LABEL(nibble_ashr_9_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0

	palignr $9, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri $0x1a, (%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif

	add	$16, %rdx
	add	$16, %r10
	jg	LABEL(nibble_ashr_9_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $9, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri $0x1a, (%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif
	add	$16, %rdx
	jmp	LABEL(loop_ashr_9_use_sse4_2)

	.p2align 4
LABEL(nibble_ashr_9_use_sse4_2):
	sub	$0x1000, %r10
	movdqa	-16(%rdi, %rdx), %xmm0
	psrldq	$9, %xmm0
	pcmpistri      $0x3a,%xmm0, %xmm0
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	cmp	%r11, %rcx
	jae	LABEL(nibble_ashr_use_sse4_2_exit)
# endif
	cmp	$6, %ecx
	ja	LABEL(loop_ashr_9_use_sse4_2)

	jmp	LABEL(nibble_ashr_use_sse4_2_exit)

/*
 *  The following cases will be handled by ashr_10
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset	 corresponding case
 *        n(6~15)          n - 6		  9(15 +(n - 6) - n)         ashr_10
 */
	.p2align 4
LABEL(ashr_10_sse4_2):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$6, %xmm2
	TOLOWER (%xmm1, %xmm2)
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes_sse4_2)
	movdqa	(%rdi), %xmm3

	UPDATE_STRNCMP_COUNTER

	pxor	%xmm0, %xmm0
	mov	$16, %rcx	/* index for loads */
	mov	$10, %r9d	/* byte position left over from less32bytes case */
	/*
	 * Setup %r10 value allows us to detect crossing a page boundary.
	 * When %r10 goes positive we have crossed a page boundary and
	 * need to do a nibble.
	 */
	lea	10(%rdi), %r10
	and	$0xfff, %r10	/* offset into 4K page */
	sub	$0x1000, %r10	/* subtract 4K pagesize */
	mov	%rcx, %rdx	/* only for offset of sse4 instruction loop*/

	.p2align 4
LABEL(loop_ashr_10_use_sse4_2):
	add	$16, %r10
	jg	LABEL(nibble_ashr_10_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $10, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri $0x1a, (%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif

	add	$16, %rdx
	add	$16, %r10
	jg	LABEL(nibble_ashr_10_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $10, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri $0x1a, (%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif
	add	$16, %rdx
	jmp	LABEL(loop_ashr_10_use_sse4_2)

	.p2align 4
LABEL(nibble_ashr_10_use_sse4_2):
	sub	$0x1000, %r10
	movdqa	-16(%rdi, %rdx), %xmm0
	psrldq	$10, %xmm0
	pcmpistri      $0x3a,%xmm0, %xmm0
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	cmp	%r11, %rcx
	jae	LABEL(nibble_ashr_use_sse4_2_exit)
# endif
	cmp	$5, %ecx
	ja	LABEL(loop_ashr_10_use_sse4_2)

	jmp	LABEL(nibble_ashr_use_sse4_2_exit)

/*
 *  The following cases will be handled by ashr_11
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset	 corresponding case
 *        n(5~15)          n - 5		  10(15 +(n - 5) - n)         ashr_11
 */
	.p2align 4
LABEL(ashr_11_sse4_2):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$5, %xmm2
	TOLOWER (%xmm1, %xmm2)
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes_sse4_2)
	movdqa	(%rdi), %xmm3

	UPDATE_STRNCMP_COUNTER

	pxor	%xmm0, %xmm0
	mov	$16, %rcx	/* index for loads */
	mov	$11, %r9d	/* byte position left over from less32bytes case */
	/*
	 * Setup %r10 value allows us to detect crossing a page boundary.
	 * When %r10 goes positive we have crossed a page boundary and
	 * need to do a nibble.
	 */
	lea	11(%rdi), %r10
	and	$0xfff, %r10	/* offset into 4K page */
	sub	$0x1000, %r10	/* subtract 4K pagesize */
	mov	%rcx, %rdx	/* only for offset of sse4 instruction loop*/

	.p2align 4
LABEL(loop_ashr_11_use_sse4_2):
	add	$16, %r10
	jg	LABEL(nibble_ashr_11_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $11, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri $0x1a, (%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif

	add	$16, %rdx
	add	$16, %r10
	jg	LABEL(nibble_ashr_11_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $11, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri $0x1a, (%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif
	add	$16, %rdx
	jmp	LABEL(loop_ashr_11_use_sse4_2)

	.p2align 4
LABEL(nibble_ashr_11_use_sse4_2):
	sub	$0x1000, %r10
	movdqa	-16(%rdi, %rdx), %xmm0
	psrldq	$11, %xmm0
	pcmpistri      $0x3a,%xmm0, %xmm0
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	cmp	%r11, %rcx
	jae	LABEL(nibble_ashr_use_sse4_2_exit)
# endif
	cmp	$4, %ecx
	ja	LABEL(loop_ashr_11_use_sse4_2)

	jmp	LABEL(nibble_ashr_use_sse4_2_exit)

/*
 *  The following cases will be handled by ashr_12
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset	 corresponding case
 *        n(4~15)          n - 4		  11(15 +(n - 4) - n)         ashr_12
 */
	.p2align 4
LABEL(ashr_12_sse4_2):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$4, %xmm2
	TOLOWER (%xmm1, %xmm2)
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes_sse4_2)
	movdqa	(%rdi), %xmm3

	UPDATE_STRNCMP_COUNTER

	pxor	%xmm0, %xmm0
	mov	$16, %rcx	/* index for loads */
	mov	$12, %r9d	/* byte position left over from less32bytes case */
	/*
	 * Setup %r10 value allows us to detect crossing a page boundary.
	 * When %r10 goes positive we have crossed a page boundary and
	 * need to do a nibble.
	 */
	lea	12(%rdi), %r10
	and	$0xfff, %r10	/* offset into 4K page */
	sub	$0x1000, %r10	/* subtract 4K pagesize */
	mov	%rcx, %rdx	/* only for offset of sse4 instruction loop*/

	.p2align 4
LABEL(loop_ashr_12_use_sse4_2):
	add	$16, %r10
	jg	LABEL(nibble_ashr_12_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $12, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri $0x1a, (%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif

	add	$16, %rdx
	add	$16, %r10
	jg	LABEL(nibble_ashr_12_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $12, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri $0x1a, (%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif
	add	$16, %rdx
	jmp	LABEL(loop_ashr_12_use_sse4_2)

	.p2align 4
LABEL(nibble_ashr_12_use_sse4_2):
	sub	$0x1000, %r10
	movdqa	-16(%rdi, %rdx), %xmm0
	psrldq	$12, %xmm0
	pcmpistri      $0x3a,%xmm0, %xmm0
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	cmp	%r11, %rcx
	jae	LABEL(nibble_ashr_use_sse4_2_exit)
# endif
	cmp	$3, %ecx
	ja	LABEL(loop_ashr_12_use_sse4_2)

	jmp	LABEL(nibble_ashr_use_sse4_2_exit)

/*
 *  The following cases will be handled by ashr_13
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset	 corresponding case
 *        n(3~15)          n - 3		  12(15 +(n - 3) - n)         ashr_13
 */
	.p2align 4
LABEL(ashr_13_sse4_2):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$3, %xmm2
	TOLOWER (%xmm1, %xmm2)
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes_sse4_2)
	movdqa	(%rdi), %xmm3

	UPDATE_STRNCMP_COUNTER

	pxor	%xmm0, %xmm0
	mov	$16, %rcx	/* index for loads */
	mov	$13, %r9d	/* byte position left over from less32bytes case */
	/*
	 * Setup %r10 value allows us to detect crossing a page boundary.
	 * When %r10 goes positive we have crossed a page boundary and
	 * need to do a nibble.
	 */
	lea	13(%rdi), %r10
	and	$0xfff, %r10	/* offset into 4K page */
	sub	$0x1000, %r10	/* subtract 4K pagesize */

	mov	%rcx, %rdx	/* only for offset of sse4 instruction loop*/

	.p2align 4
LABEL(loop_ashr_13_use_sse4_2):
	add	$16, %r10
	jg	LABEL(nibble_ashr_13_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $13, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri $0x1a, (%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif

	add	$16, %rdx
	add	$16, %r10
	jg	LABEL(nibble_ashr_13_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $13, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri $0x1a, (%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif
	add	$16, %rdx
	jmp	LABEL(loop_ashr_13_use_sse4_2)

	.p2align 4
LABEL(nibble_ashr_13_use_sse4_2):
	sub	$0x1000, %r10
	movdqa	-16(%rdi, %rdx), %xmm0
	psrldq	$13, %xmm0
	pcmpistri      $0x3a,%xmm0, %xmm0
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	cmp	%r11, %rcx
	jae	LABEL(nibble_ashr_use_sse4_2_exit)
# endif
	cmp	$2, %ecx
	ja	LABEL(loop_ashr_13_use_sse4_2)

	jmp	LABEL(nibble_ashr_use_sse4_2_exit)

/*
 *  The following cases will be handled by ashr_14
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset	 corresponding case
 *        n(2~15)          n - 2		  13(15 +(n - 2) - n)         ashr_14
 */
	.p2align 4
LABEL(ashr_14_sse4_2):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq  $2, %xmm2
	TOLOWER (%xmm1, %xmm2)
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes_sse4_2)
	movdqa	(%rdi), %xmm3

	UPDATE_STRNCMP_COUNTER

	pxor	%xmm0, %xmm0
	mov	$16, %rcx	/* index for loads */
	mov	$14, %r9d	/* byte position left over from less32bytes case */
	/*
	 * Setup %r10 value allows us to detect crossing a page boundary.
	 * When %r10 goes positive we have crossed a page boundary and
	 * need to do a nibble.
	 */
	lea	14(%rdi), %r10
	and	$0xfff, %r10	/* offset into 4K page */
	sub	$0x1000, %r10	/* subtract 4K pagesize */

	mov	%rcx, %rdx	/* only for offset of sse4 instruction loop*/

	.p2align 4
LABEL(loop_ashr_14_use_sse4_2):
	add	$16, %r10
	jg	LABEL(nibble_ashr_14_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $14, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri $0x1a, (%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif

	add	$16, %rdx
	add	$16, %r10
	jg	LABEL(nibble_ashr_14_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $14, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri $0x1a, (%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif
	add	$16, %rdx
	jmp	LABEL(loop_ashr_14_use_sse4_2)

	.p2align 4
LABEL(nibble_ashr_14_use_sse4_2):
	sub	$0x1000, %r10
	movdqa	-16(%rdi, %rdx), %xmm0
	psrldq	$14, %xmm0
	pcmpistri      $0x3a,%xmm0, %xmm0
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	cmp	%r11, %rcx
	jae	LABEL(nibble_ashr_use_sse4_2_exit)
# endif
	cmp	$1, %ecx
	ja	LABEL(loop_ashr_14_use_sse4_2)

	jmp	LABEL(nibble_ashr_use_sse4_2_exit)

/*
 *  The following cases will be handled by ashr_15
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset	 corresponding case
 *        n(1~15)          n - 1		  14(15 +(n - 1) - n)         ashr_15
 */
	.p2align 4
LABEL(ashr_15_sse4_2):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$1, %xmm2
	TOLOWER (%xmm1, %xmm2)
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes_sse4_2)

	movdqa	(%rdi), %xmm3

	UPDATE_STRNCMP_COUNTER

	pxor	%xmm0, %xmm0
	mov	$16, %rcx	/* index for loads */
	mov	$15, %r9d	/* byte position left over from less32bytes case */
	/*
	 * Setup %r10 value allows us to detect crossing a page boundary.
	 * When %r10 goes positive we have crossed a page boundary and
	 * need to do a nibble.
	 */
	lea	15(%rdi), %r10
	and	$0xfff, %r10	/* offset into 4K page */

	sub	$0x1000, %r10	/* subtract 4K pagesize */

	mov	%rcx, %rdx	/* only for offset of sse4 instruction loop*/

	.p2align 4
LABEL(loop_ashr_15_use_sse4_2):
	add	$16, %r10
	jg	LABEL(nibble_ashr_15_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $15, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri $0x1a, (%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif

	add	$16, %rdx
	add	$16, %r10
	jg	LABEL(nibble_ashr_15_use_sse4_2)

	movdqa	(%rdi, %rdx), %xmm0
	palignr $15, -16(%rdi, %rdx), %xmm0
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri $0x1a, (%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	jbe	LABEL(use_sse4_2_exit)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif
	add	$16, %rdx
	jmp	LABEL(loop_ashr_15_use_sse4_2)

	.p2align 4
LABEL(nibble_ashr_15_use_sse4_2):
	sub	$0x1000, %r10
	movdqa	-16(%rdi, %rdx), %xmm0
	psrldq	$15, %xmm0
	pcmpistri      $0x3a,%xmm0, %xmm0
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	cmp	%r11, %rcx
	jae	LABEL(nibble_ashr_use_sse4_2_exit)
# endif
	cmp	$0, %ecx
	ja	LABEL(loop_ashr_15_use_sse4_2)

LABEL(nibble_ashr_use_sse4_2_exit):
# if !defined USE_AS_STRCASECMP_L && !defined USE_AS_STRNCASECMP_L
	pcmpistri      $0x1a,(%rsi,%rdx), %xmm0
# else
	movdqa	(%rsi,%rdx), %xmm1
	TOLOWER (%xmm0, %xmm1)
	pcmpistri $0x1a, %xmm1, %xmm0
# endif
	.p2align 4
LABEL(use_sse4_2_exit):
	jnc	LABEL(strcmp_exitz_sse4_2)
# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	%rcx, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif
	add	%rcx, %rdx
	lea	-16(%rdi, %r9), %rdi
	movzbl	(%rdi, %rdx), %eax
	movzbl	(%rsi, %rdx), %edx
	test	%r8d, %r8d
	jz	LABEL(use_sse4_2_ret_sse4_2)
	xchg	%eax, %edx
LABEL(use_sse4_2_ret_sse4_2):
# if defined USE_AS_STRCASECMP_L || defined USE_AS_STRNCASECMP_L
	leaq	_nl_C_LC_CTYPE_tolower+128*4(%rip), %rcx
	movl	(%rcx,%rdx,4), %edx
	movl	(%rcx,%rax,4), %eax
# endif

	sub	%edx, %eax
	ret

LABEL(less32bytes_sse4_2):
	lea	(%rdi, %rax), %rdi	/* locate the exact address for first operand(rdi) */
	lea	(%rsi, %rcx), %rsi	/* locate the exact address for second operand(rsi) */
	test	%r8d, %r8d
	jz	LABEL(ret_sse4_2)
	xchg	%rsi, %rdi		/* recover original order according to flag(%r8d) */

	.p2align 4
LABEL(ret_sse4_2):
LABEL(less16bytes_sse4_2):
	bsf	%rdx, %rdx		/* find and store bit index in %rdx */

# if defined USE_AS_STRNCMP || defined USE_AS_STRNCASECMP_L
	sub	%rdx, %r11
	jbe	LABEL(strcmp_exitz_sse4_2)
# endif
	movzbl	(%rsi, %rdx), %ecx
	movzbl	(%rdi, %rdx), %eax

# if defined USE_AS_STRCASECMP_L || defined USE_AS_STRNCASECMP_L
	leaq	_nl_C_LC_CTYPE_tolower+128*4(%rip), %rdx
	movl	(%rdx,%rcx,4), %ecx
	movl	(%rdx,%rax,4), %eax
# endif

	sub	%ecx, %eax
	ret

LABEL(strcmp_exitz_sse4_2):
	xor	%eax, %eax
	ret

	.p2align 4
	// XXX Same as code above
LABEL(Byte0_sse4_2):
	movzx	(%rsi), %ecx
	movzx	(%rdi), %eax

# if defined USE_AS_STRCASECMP_L || defined USE_AS_STRNCASECMP_L
	leaq	_nl_C_LC_CTYPE_tolower+128*4(%rip), %rdx
	movl	(%rdx,%rcx,4), %ecx
	movl	(%rdx,%rax,4), %eax
# endif

	sub	%ecx, %eax
	ret
	cfi_endproc
	.size	STRCMP_SSE42, .-STRCMP_SSE42

# undef UCLOW_reg
# undef UCHIGH_reg
# undef LCQWORD_reg
# undef TOLOWER

	/* Put all SSE 4.2 functions together.  */
	.section .rodata.sse4.2,"a",@progbits
	.p2align 3
LABEL(unaligned_table_sse4_2):
	.int	LABEL(ashr_1_sse4_2) - LABEL(unaligned_table_sse4_2)
	.int	LABEL(ashr_2_sse4_2) - LABEL(unaligned_table_sse4_2)
	.int	LABEL(ashr_3_sse4_2) - LABEL(unaligned_table_sse4_2)
	.int	LABEL(ashr_4_sse4_2) - LABEL(unaligned_table_sse4_2)
	.int	LABEL(ashr_5_sse4_2) - LABEL(unaligned_table_sse4_2)
	.int	LABEL(ashr_6_sse4_2) - LABEL(unaligned_table_sse4_2)
	.int	LABEL(ashr_7_sse4_2) - LABEL(unaligned_table_sse4_2)
	.int	LABEL(ashr_8_sse4_2) - LABEL(unaligned_table_sse4_2)
	.int	LABEL(ashr_9_sse4_2) - LABEL(unaligned_table_sse4_2)
	.int	LABEL(ashr_10_sse4_2) - LABEL(unaligned_table_sse4_2)
	.int	LABEL(ashr_11_sse4_2) - LABEL(unaligned_table_sse4_2)
	.int	LABEL(ashr_12_sse4_2) - LABEL(unaligned_table_sse4_2)
	.int	LABEL(ashr_13_sse4_2) - LABEL(unaligned_table_sse4_2)
	.int	LABEL(ashr_14_sse4_2) - LABEL(unaligned_table_sse4_2)
	.int	LABEL(ashr_15_sse4_2) - LABEL(unaligned_table_sse4_2)
	.int	LABEL(ashr_0_sse4_2) - LABEL(unaligned_table_sse4_2)


# undef ENTRY
# define ENTRY(name) \
	.type STRCMP_SSE2, @function; \
	.align 16; \
	STRCMP_SSE2: cfi_startproc; \
	CALL_MCOUNT
# undef END
# define END(name) \
	cfi_endproc; .size STRCMP_SSE2, .-STRCMP_SSE2

# ifdef USE_AS_STRCASECMP_L
#  define ENTRY2(name) \
	.type __strcasecmp_sse2, @function; \
	.align 16; \
	__strcasecmp_sse2: cfi_startproc; \
	CALL_MCOUNT
#  define END2(name) \
	cfi_endproc; .size __strcasecmp_sse2, .-__strcasecmp_sse2
# endif

# ifdef USE_AS_STRNCASECMP_L
#  define ENTRY2(name) \
	.type __strncasecmp_sse2, @function; \
	.align 16; \
	__strncasecmp_sse2: cfi_startproc; \
	CALL_MCOUNT
#  define END2(name) \
	cfi_endproc; .size __strncasecmp_sse2, .-__strncasecmp_sse2
# endif

# undef libc_hidden_builtin_def
/* It doesn't make sense to send libc-internal strcmp calls through a PLT.
   The speedup we get from using SSE4.2 instruction is likely eaten away
   by the indirect call in the PLT.  */
# define libc_hidden_builtin_def(name) \
	.globl __GI_STRCMP; __GI_STRCMP = STRCMP_SSE2
#endif

#include "../strcmp.S"
