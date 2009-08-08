/* Highly optimized version for x86-64.
   Copyright (C) 1999, 2000, 2002, 2003, 2005, 2009
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Based on i686 version contributed by Ulrich Drepper
   <drepper@cygnus.com>, 1999.
   Updated with SSE2 support contributed by Intel Corporation.

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
#include "asm-syntax.h"
#include "bp-sym.h"
#include "bp-asm.h"

#undef UPDATE_STRNCMP_COUNTER

#ifndef LABEL
#define LABEL(l) L(l)
#endif

#ifdef USE_AS_STRNCMP
/* Since the counter, %r11, is unsigned, we branch to strcmp_exitz
   if the new counter > the old one or is 0.  */
# define UPDATE_STRNCMP_COUNTER				\
	/* calculate left number to compare */		\
	lea	-16(%rcx, %r11), %r9;			\
	cmp	%r9, %r11;				\
	jb	LABEL(strcmp_exitz);			\
	test	%r9, %r9;				\
	je	LABEL(strcmp_exitz);			\
	mov	%r9, %r11

#else
# define UPDATE_STRNCMP_COUNTER
# ifndef STRCMP
#  define STRCMP strcmp
# endif
#endif

#ifndef USE_SSSE3
	.text
#else
        .section .text.ssse3,"ax",@progbits
#endif

ENTRY (BP_SYM (STRCMP))
#ifdef NOT_IN_libc
/* Simple version since we can't use SSE registers in ld.so.  */
L(oop):	movb	(%rdi), %al
	cmpb	(%rsi), %al
	jne	L(neq)
	incq	%rdi
	incq	%rsi
	testb	%al, %al
	jnz	L(oop)

	xorl	%eax, %eax
	ret

L(neq):	movl	$1, %eax
	movl	$-1, %ecx
	cmovbl	%ecx, %eax
	ret
END (BP_SYM (STRCMP))
#else	/* NOT_IN_libc */
/*
 * This implementation uses SSE to compare up to 16 bytes at a time.
 */
#ifdef USE_AS_STRNCMP
	test	%rdx, %rdx
	je	LABEL(strcmp_exitz)
	cmp	$1, %rdx
	je	LABEL(Byte0)
	mov	%rdx, %r11
#endif
	mov	%esi, %ecx
	mov	%edi, %eax
/* Use 64bit AND here to avoid long NOP padding.  */
	and	$0x3f, %rcx		/* rsi alignment in cache line */
	and	$0x3f, %rax		/* rdi alignment in cache line */
	cmp	$0x30, %ecx
	ja	LABEL(crosscache)	/* rsi: 16-byte load will cross cache line */
	cmp	$0x30, %eax
	ja	LABEL(crosscache)	/* rdi: 16-byte load will cross cache line */
	movlpd	(%rdi), %xmm1
	movlpd	(%rsi), %xmm2
	movhpd	8(%rdi), %xmm1
	movhpd	8(%rsi), %xmm2
	pxor	%xmm0, %xmm0		/* clear %xmm0 for null char checks */
	pcmpeqb	%xmm1, %xmm0		/* Any null chars? */
	pcmpeqb	%xmm2, %xmm1		/* compare first 16 bytes for equality */
	psubb	%xmm0, %xmm1		/* packed sub of comparison results*/
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx		/* if first 16 bytes are same, edx == 0xffff */
	jnz	LABEL(less16bytes)	/* If not, find different value or null char */
#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)	/* finish comparision */
#endif
	add	$16, %rsi		/* prepare to search next 16 bytes */
	add	$16, %rdi		/* prepare to search next 16 bytes */

	/*
	 * Determine source and destination string offsets from 16-byte alignment.
	 * Use relative offset difference between the two to determine which case
	 * below to use.
	 */
	.p2align 4
LABEL(crosscache):
	and	$0xfffffffffffffff0, %rsi	/* force %rsi is 16 byte aligned */
	and	$0xfffffffffffffff0, %rdi	/* force %rdi is 16 byte aligned */
	mov	$0xffff, %edx			/* for equivalent offset */
	xor	%r8d, %r8d
	and	$0xf, %ecx			/* offset of rsi */
	and	$0xf, %eax			/* offset of rdi */
	cmp	%eax, %ecx
	je	LABEL(ashr_0)			/* rsi and rdi relative offset same */
	ja	LABEL(bigger)
	mov	%edx, %r8d			/* r8d is offset flag for exit tail */
	xchg	%ecx, %eax
	xchg	%rsi, %rdi
LABEL(bigger):
	lea	15(%rax), %r9
	sub	%rcx, %r9
	lea	LABEL(unaligned_table)(%rip), %r10
	movslq	(%r10, %r9,4), %r9
	lea	(%r10, %r9), %r10
	jmp	*%r10				/* jump to corresponding case */

/*
 * The following cases will be handled by ashr_0
 *  rcx(offset of rsi)  rax(offset of rdi)  relative offset  corresponding case
 *        n(0~15)            n(0~15)           15(15+ n-n)         ashr_0
 */
	.p2align 4
LABEL(ashr_0):

	movdqa	(%rsi), %xmm1
	pxor	%xmm0, %xmm0			/* clear %xmm0 for null char check */
	pcmpeqb	%xmm1, %xmm0			/* Any null chars? */
	pcmpeqb	(%rdi), %xmm1			/* compare 16 bytes for equality */
	psubb	%xmm0, %xmm1			/* packed sub of comparison results*/
	pmovmskb %xmm1, %r9d
	shr	%cl, %edx			/* adjust 0xffff for offset */
	shr	%cl, %r9d			/* adjust for 16-byte offset */
	sub	%r9d, %edx
	/*
	 * edx must be the same with r9d if in left byte (16-rcx) is equal to
	 * the start from (16-rax) and no null char was seen.
	 */
	jne	LABEL(less32bytes)		/* mismatch or null char */
	UPDATE_STRNCMP_COUNTER
	mov	$16, %rcx
	mov	$16, %r9
	pxor	%xmm0, %xmm0			/* clear xmm0, may have changed above */

	/*
	 * Now both strings are aligned at 16-byte boundary. Loop over strings
	 * checking 32-bytes per iteration.
	 */
	.p2align 4
LABEL(loop_ashr_0):
	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)		/* mismatch or null char seen */

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif
	add	$16, %rcx
	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)
#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif
	add	$16, %rcx
	jmp	LABEL(loop_ashr_0)

/*
 * The following cases will be handled by ashr_1
 * rcx(offset of rsi)  rax(offset of rdi)   relative offset   	corresponding case
 *        n(15)            n -15            0(15 +(n-15) - n)         ashr_1
 */
	.p2align 4
LABEL(ashr_1):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0		/* Any null chars? */
	pslldq	$15, %xmm2		/* shift first string to align with second */
	pcmpeqb	%xmm1, %xmm2		/* compare 16 bytes for equality */
	psubb	%xmm0, %xmm2		/* packed sub of comparison results*/
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx		/* adjust 0xffff for offset */
	shr	%cl, %r9d		/* adjust for 16-byte offset */
	sub	%r9d, %edx
	jnz	LABEL(less32bytes)	/* mismatch or null char seen */
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

	.p2align 4
LABEL(loop_ashr_1):
	add	$16, %r10
	jg	LABEL(nibble_ashr_1)	/* cross page boundary */

LABEL(gobble_ashr_1):
	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4		 /* store for next cycle */

#ifndef USE_SSSE3
	psrldq	$1, %xmm3
	pslldq	$15, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$1, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif
	add	$16, %rcx
	movdqa	%xmm4, %xmm3

	add	$16, %r10
	jg	LABEL(nibble_ashr_1)	/* cross page boundary */

	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4		/* store for next cycle */

#ifndef USE_SSSE3
	psrldq	$1, %xmm3
	pslldq	$15, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$1, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif
	add	$16, %rcx
	movdqa	%xmm4, %xmm3
	jmp	LABEL(loop_ashr_1)

	/*
	 * Nibble avoids loads across page boundary. This is to avoid a potential
	 * access into unmapped memory.
	 */
	.p2align 4
LABEL(nibble_ashr_1):
	pcmpeqb	%xmm3, %xmm0		 /* check nibble for null char*/
	pmovmskb %xmm0, %edx
	test	$0xfffe, %edx
	jnz	LABEL(ashr_1_exittail)	/* find null char*/

#ifdef USE_AS_STRNCMP
	cmp	$14, %r11
	jbe	LABEL(ashr_1_exittail)
#endif

	pxor	%xmm0, %xmm0
	sub	$0x1000, %r10		/* substract 4K from %r10 */
	jmp	LABEL(gobble_ashr_1)

	/*
	 * Once find null char, determine if there is a string mismatch
	 * before the null char.
	 */
	.p2align 4
LABEL(ashr_1_exittail):
	movdqa	(%rsi, %rcx), %xmm1
	psrldq	$1, %xmm0
	psrldq	$1, %xmm3
	jmp	LABEL(aftertail)

/*
 * The following cases will be handled by ashr_2
 * rcx(offset of rsi)  rax(offset of rdi)   relative offset   	corresponding case
 *        n(14~15)            n -14         1(15 +(n-14) - n)         ashr_2
 */
	.p2align 4
LABEL(ashr_2):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$14, %xmm2
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes)
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

	.p2align 4
LABEL(loop_ashr_2):
	add	$16, %r10
	jg	LABEL(nibble_ashr_2)

LABEL(gobble_ashr_2):
	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$2, %xmm3
	pslldq	$14, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$2, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3

	add	$16, %r10
	jg	LABEL(nibble_ashr_2)	/* cross page boundary */

	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$2, %xmm3
	pslldq	$14, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$2, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3
	jmp	LABEL(loop_ashr_2)

	.p2align 4
LABEL(nibble_ashr_2):
	pcmpeqb	%xmm3, %xmm0		/* check nibble for null char */
	pmovmskb %xmm0, %edx
	test	$0xfffc, %edx
	jnz	LABEL(ashr_2_exittail)

#ifdef USE_AS_STRNCMP
	cmp	$13, %r11
	jbe	LABEL(ashr_2_exittail)
#endif

	pxor	%xmm0, %xmm0
	sub	$0x1000, %r10
	jmp	LABEL(gobble_ashr_2)

	.p2align 4
LABEL(ashr_2_exittail):
	movdqa	(%rsi, %rcx), %xmm1
	psrldq	$2, %xmm0
	psrldq	$2, %xmm3
	jmp	LABEL(aftertail)

/*
 * The following cases will be handled by ashr_3
 *  rcx(offset of rsi)  rax(offset of rdi)  relative offset	 corresponding case
 *        n(13~15)            n -13         2(15 +(n-13) - n)         ashr_3
 */
	.p2align 4
LABEL(ashr_3):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$13, %xmm2
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes)
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

	.p2align 4
LABEL(loop_ashr_3):
	add	$16, %r10
	jg	LABEL(nibble_ashr_3)

LABEL(gobble_ashr_3):
	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$3, %xmm3
	pslldq	$13, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$3, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3

	add	$16, %r10
	jg	LABEL(nibble_ashr_3)	/* cross page boundary */

	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$3, %xmm3
	pslldq	$13, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$3, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3
	jmp	LABEL(loop_ashr_3)

	.p2align 4
LABEL(nibble_ashr_3):
	pcmpeqb	%xmm3, %xmm0		/* check nibble for null char */
	pmovmskb %xmm0, %edx
	test	$0xfff8, %edx
	jnz	LABEL(ashr_3_exittail)

#ifdef USE_AS_STRNCMP
	cmp	$12, %r11
	jbe	LABEL(ashr_3_exittail)
#endif

	pxor	%xmm0, %xmm0
	sub	$0x1000, %r10
	jmp	LABEL(gobble_ashr_3)

	.p2align 4
LABEL(ashr_3_exittail):
	movdqa	(%rsi, %rcx), %xmm1
	psrldq	$3, %xmm0
	psrldq	$3, %xmm3
	jmp	LABEL(aftertail)

/*
 * The following cases will be handled by ashr_4
 *  rcx(offset of rsi)  rax(offset of rdi)  relative offset	 corresponding case
 *        n(12~15)            n -12         3(15 +(n-12) - n)         ashr_4
 */
	.p2align 4
LABEL(ashr_4):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$12, %xmm2
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes)
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

	.p2align 4
LABEL(loop_ashr_4):
	add	$16, %r10
	jg	LABEL(nibble_ashr_4)

LABEL(gobble_ashr_4):
	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$4, %xmm3
	pslldq	$12, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$4, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3

	add	$16, %r10
	jg	LABEL(nibble_ashr_4)	/* cross page boundary */

	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$4, %xmm3
	pslldq	$12, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$4, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3
	jmp	LABEL(loop_ashr_4)

	.p2align 4
LABEL(nibble_ashr_4):
	pcmpeqb	%xmm3, %xmm0		/* check nibble for null char */
	pmovmskb %xmm0, %edx
	test	$0xfff0, %edx
	jnz	LABEL(ashr_4_exittail)

#ifdef USE_AS_STRNCMP
	cmp	$11, %r11
	jbe	LABEL(ashr_4_exittail)
#endif

	pxor	%xmm0, %xmm0
	sub	$0x1000, %r10
	jmp	LABEL(gobble_ashr_4)

	.p2align 4
LABEL(ashr_4_exittail):
	movdqa	(%rsi, %rcx), %xmm1
	psrldq	$4, %xmm0
	psrldq	$4, %xmm3
	jmp	LABEL(aftertail)

/*
 * The following cases will be handled by ashr_5
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset      corresponding case
 *        n(11~15)          n - 11      	  4(15 +(n-11) - n)         ashr_5
 */
	.p2align 4
LABEL(ashr_5):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$11, %xmm2
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes)
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

	.p2align 4
LABEL(loop_ashr_5):
	add	$16, %r10
	jg	LABEL(nibble_ashr_5)

LABEL(gobble_ashr_5):
	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$5, %xmm3
	pslldq	$11, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$5, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3

	add	$16, %r10
	jg	LABEL(nibble_ashr_5)	/* cross page boundary */

	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$5, %xmm3
	pslldq	$11, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$5, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3
	jmp	LABEL(loop_ashr_5)

	.p2align 4
LABEL(nibble_ashr_5):
	pcmpeqb	%xmm3, %xmm0		/* check nibble for null char */
	pmovmskb %xmm0, %edx
	test	$0xffe0, %edx
	jnz	LABEL(ashr_5_exittail)

#ifdef USE_AS_STRNCMP
	cmp	$10, %r11
	jbe	LABEL(ashr_5_exittail)
#endif

	pxor	%xmm0, %xmm0
	sub	$0x1000, %r10
	jmp	LABEL(gobble_ashr_5)

	.p2align 4
LABEL(ashr_5_exittail):
	movdqa	(%rsi, %rcx), %xmm1
	psrldq	$5, %xmm0
	psrldq	$5, %xmm3
	jmp	LABEL(aftertail)

/*
 * The following cases will be handled by ashr_6
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset      corresponding case
 *        n(10~15)          n - 10      	  5(15 +(n-10) - n)         ashr_6
 */
	.p2align 4
LABEL(ashr_6):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$10, %xmm2
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes)
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

	.p2align 4
LABEL(loop_ashr_6):
	add	$16, %r10
	jg	LABEL(nibble_ashr_6)

LABEL(gobble_ashr_6):
	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$6, %xmm3
	pslldq	$10, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$6, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3

	add	$16, %r10
	jg	LABEL(nibble_ashr_6)	/* cross page boundary */

	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$6, %xmm3
	pslldq	$10, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$6, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3
	jmp	LABEL(loop_ashr_6)

	.p2align 4
LABEL(nibble_ashr_6):
	pcmpeqb	%xmm3, %xmm0		/* check nibble for null char */
	pmovmskb %xmm0, %edx
	test	$0xffc0, %edx
	jnz	LABEL(ashr_6_exittail)

#ifdef USE_AS_STRNCMP
	cmp	$9, %r11
	jbe	LABEL(ashr_6_exittail)
#endif

	pxor	%xmm0, %xmm0
	sub	$0x1000, %r10
	jmp	LABEL(gobble_ashr_6)

	.p2align 4
LABEL(ashr_6_exittail):
	movdqa	(%rsi, %rcx), %xmm1
	psrldq	$6, %xmm0
	psrldq	$6, %xmm3
	jmp	LABEL(aftertail)

/*
 * The following cases will be handled by ashr_7
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset      corresponding case
 *        n(9~15)          n - 9      	        6(15 +(n - 9) - n)         ashr_7
 */
	.p2align 4
LABEL(ashr_7):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$9, %xmm2
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes)
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

	.p2align 4
LABEL(loop_ashr_7):
	add	$16, %r10
	jg	LABEL(nibble_ashr_7)

LABEL(gobble_ashr_7):
	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$7, %xmm3
	pslldq	$9, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$7, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3

	add	$16, %r10
	jg	LABEL(nibble_ashr_7)	/* cross page boundary */

	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$7, %xmm3
	pslldq	$9, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$7, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3
	jmp	LABEL(loop_ashr_7)

	.p2align 4
LABEL(nibble_ashr_7):
	pcmpeqb	%xmm3, %xmm0		/* check nibble for null char */
	pmovmskb %xmm0, %edx
	test	$0xff80, %edx
	jnz	LABEL(ashr_7_exittail)

#ifdef USE_AS_STRNCMP
	cmp	$8, %r11
	jbe	LABEL(ashr_7_exittail)
#endif

	pxor	%xmm0, %xmm0
	sub	$0x1000, %r10
	jmp	LABEL(gobble_ashr_7)

	.p2align 4
LABEL(ashr_7_exittail):
	movdqa	(%rsi, %rcx), %xmm1
	psrldq	$7, %xmm0
	psrldq	$7, %xmm3
	jmp	LABEL(aftertail)

/*
 *  The following cases will be handled by ashr_8
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset	 corresponding case
 *        n(8~15)          n - 8      	        7(15 +(n - 8) - n)         ashr_8
 */
	.p2align 4
LABEL(ashr_8):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$8, %xmm2
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes)
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

	.p2align 4
LABEL(loop_ashr_8):
	add	$16, %r10
	jg	LABEL(nibble_ashr_8)

LABEL(gobble_ashr_8):
	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$8, %xmm3
	pslldq	$8, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$8, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3

	add	$16, %r10
	jg	LABEL(nibble_ashr_8)	/* cross page boundary */

	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$8, %xmm3
	pslldq	$8, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$8, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3
	jmp	LABEL(loop_ashr_8)

	.p2align 4
LABEL(nibble_ashr_8):
	pcmpeqb	%xmm3, %xmm0		/* check nibble for null char */
	pmovmskb %xmm0, %edx
	test	$0xff00, %edx
	jnz	LABEL(ashr_8_exittail)

#ifdef USE_AS_STRNCMP
	cmp	$7, %r11
	jbe	LABEL(ashr_8_exittail)
#endif

	pxor	%xmm0, %xmm0
	sub	$0x1000, %r10
	jmp	LABEL(gobble_ashr_8)

	.p2align 4
LABEL(ashr_8_exittail):
	movdqa	(%rsi, %rcx), %xmm1
	psrldq	$8, %xmm0
	psrldq	$8, %xmm3
	jmp	LABEL(aftertail)

/*
 *  The following cases will be handled by ashr_9
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset	 corresponding case
 *        n(7~15)          n - 7      	        8(15 +(n - 7) - n)         ashr_9
 */
	.p2align 4
LABEL(ashr_9):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$7, %xmm2
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes)
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

	.p2align 4
LABEL(loop_ashr_9):
	add	$16, %r10
	jg	LABEL(nibble_ashr_9)

LABEL(gobble_ashr_9):
	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$9, %xmm3
	pslldq	$7, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$9, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3

	add	$16, %r10
	jg	LABEL(nibble_ashr_9)	/* cross page boundary */

	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$9, %xmm3
	pslldq	$7, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$9, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3		/* store for next cycle */
	jmp	LABEL(loop_ashr_9)

	.p2align 4
LABEL(nibble_ashr_9):
	pcmpeqb	%xmm3, %xmm0		/* check nibble for null char */
	pmovmskb %xmm0, %edx
	test	$0xfe00, %edx
	jnz	LABEL(ashr_9_exittail)

#ifdef USE_AS_STRNCMP
	cmp	$6, %r11
	jbe	LABEL(ashr_9_exittail)
#endif

	pxor	%xmm0, %xmm0
	sub	$0x1000, %r10
	jmp	LABEL(gobble_ashr_9)

	.p2align 4
LABEL(ashr_9_exittail):
	movdqa	(%rsi, %rcx), %xmm1
	psrldq	$9, %xmm0
	psrldq	$9, %xmm3
	jmp	LABEL(aftertail)

/*
 *  The following cases will be handled by ashr_10
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset	 corresponding case
 *        n(6~15)          n - 6      	        9(15 +(n - 6) - n)         ashr_10
 */
	.p2align 4
LABEL(ashr_10):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$6, %xmm2
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes)
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

	.p2align 4
LABEL(loop_ashr_10):
	add	$16, %r10
	jg	LABEL(nibble_ashr_10)

LABEL(gobble_ashr_10):
	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$10, %xmm3
	pslldq	$6, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$10, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3

	add	$16, %r10
	jg	LABEL(nibble_ashr_10)	/* cross page boundary */

	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$10, %xmm3
	pslldq	$6, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$10, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3
	jmp	LABEL(loop_ashr_10)

	.p2align 4
LABEL(nibble_ashr_10):
	pcmpeqb	%xmm3, %xmm0		/* check nibble for null char */
	pmovmskb %xmm0, %edx
	test	$0xfc00, %edx
	jnz	LABEL(ashr_10_exittail)

#ifdef USE_AS_STRNCMP
	cmp	$5, %r11
	jbe	LABEL(ashr_10_exittail)
#endif

	pxor	%xmm0, %xmm0
	sub	$0x1000, %r10
	jmp	LABEL(gobble_ashr_10)

	.p2align 4
LABEL(ashr_10_exittail):
	movdqa	(%rsi, %rcx), %xmm1
	psrldq	$10, %xmm0
	psrldq	$10, %xmm3
	jmp	LABEL(aftertail)

/*
 *  The following cases will be handled by ashr_11
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset	 corresponding case
 *        n(5~15)          n - 5      	        10(15 +(n - 5) - n)         ashr_11
 */
	.p2align 4
LABEL(ashr_11):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$5, %xmm2
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes)
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

	.p2align 4
LABEL(loop_ashr_11):
	add	$16, %r10
	jg	LABEL(nibble_ashr_11)

LABEL(gobble_ashr_11):
	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$11, %xmm3
	pslldq	$5, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$11, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3

	add	$16, %r10
	jg	LABEL(nibble_ashr_11)	/* cross page boundary */

	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$11, %xmm3
	pslldq	$5, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$11, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3
	jmp	LABEL(loop_ashr_11)

	.p2align 4
LABEL(nibble_ashr_11):
	pcmpeqb	%xmm3, %xmm0		/* check nibble for null char */
	pmovmskb %xmm0, %edx
	test	$0xf800, %edx
	jnz	LABEL(ashr_11_exittail)

#ifdef USE_AS_STRNCMP
	cmp	$4, %r11
	jbe	LABEL(ashr_11_exittail)
#endif

	pxor	%xmm0, %xmm0
	sub	$0x1000, %r10
	jmp	LABEL(gobble_ashr_11)

	.p2align 4
LABEL(ashr_11_exittail):
	movdqa	(%rsi, %rcx), %xmm1
	psrldq	$11, %xmm0
	psrldq	$11, %xmm3
	jmp	LABEL(aftertail)

/*
 *  The following cases will be handled by ashr_12
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset	 corresponding case
 *        n(4~15)          n - 4      	        11(15 +(n - 4) - n)         ashr_12
 */
	.p2align 4
LABEL(ashr_12):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$4, %xmm2
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes)
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

	.p2align 4
LABEL(loop_ashr_12):
	add	$16, %r10
	jg	LABEL(nibble_ashr_12)

LABEL(gobble_ashr_12):
	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$12, %xmm3
	pslldq	$4, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$12, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3

	add	$16, %r10
	jg	LABEL(nibble_ashr_12)	/* cross page boundary */

	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$12, %xmm3
	pslldq	$4, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$12, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3
	jmp	LABEL(loop_ashr_12)

	.p2align 4
LABEL(nibble_ashr_12):
	pcmpeqb	%xmm3, %xmm0		/* check nibble for null char */
	pmovmskb %xmm0, %edx
	test	$0xf000, %edx
	jnz	LABEL(ashr_12_exittail)

#ifdef USE_AS_STRNCMP
	cmp	$3, %r11
	jbe	LABEL(ashr_12_exittail)
#endif

	pxor	%xmm0, %xmm0
	sub	$0x1000, %r10
	jmp	LABEL(gobble_ashr_12)

	.p2align 4
LABEL(ashr_12_exittail):
	movdqa	(%rsi, %rcx), %xmm1
	psrldq	$12, %xmm0
	psrldq	$12, %xmm3
	jmp	LABEL(aftertail)

/*
 *  The following cases will be handled by ashr_13
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset	 corresponding case
 *        n(3~15)          n - 3      	        12(15 +(n - 3) - n)         ashr_13
 */
	.p2align 4
LABEL(ashr_13):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$3, %xmm2
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes)
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

	.p2align 4
LABEL(loop_ashr_13):
	add	$16, %r10
	jg	LABEL(nibble_ashr_13)

LABEL(gobble_ashr_13):
	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$13, %xmm3
	pslldq	$3, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$13, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3

	add	$16, %r10
	jg	LABEL(nibble_ashr_13)	/* cross page boundary */

	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$13, %xmm3
	pslldq	$3, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$13, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3
	jmp	LABEL(loop_ashr_13)

	.p2align 4
LABEL(nibble_ashr_13):
	pcmpeqb	%xmm3, %xmm0		/* check nibble for null char */
	pmovmskb %xmm0, %edx
	test	$0xe000, %edx
	jnz	LABEL(ashr_13_exittail)

#ifdef USE_AS_STRNCMP
	cmp	$2, %r11
	jbe	LABEL(ashr_13_exittail)
#endif

	pxor	%xmm0, %xmm0
	sub	$0x1000, %r10
	jmp	LABEL(gobble_ashr_13)

	.p2align 4
LABEL(ashr_13_exittail):
	movdqa	(%rsi, %rcx), %xmm1
	psrldq  $13, %xmm0
	psrldq  $13, %xmm3
	jmp	LABEL(aftertail)

/*
 *  The following cases will be handled by ashr_14
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset	 corresponding case
 *        n(2~15)          n - 2      	        13(15 +(n - 2) - n)         ashr_14
 */
	.p2align 4
LABEL(ashr_14):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq  $2, %xmm2
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes)
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

	.p2align 4
LABEL(loop_ashr_14):
	add	$16, %r10
	jg	LABEL(nibble_ashr_14)

LABEL(gobble_ashr_14):
	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$14, %xmm3
	pslldq	$2, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$14, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3

	add	$16, %r10
	jg	LABEL(nibble_ashr_14)	/* cross page boundary */

	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$14, %xmm3
	pslldq	$2, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$14, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3
	jmp	LABEL(loop_ashr_14)

	.p2align 4
LABEL(nibble_ashr_14):
	pcmpeqb	%xmm3, %xmm0		/* check nibble for null char */
	pmovmskb %xmm0, %edx
	test	$0xc000, %edx
	jnz	LABEL(ashr_14_exittail)

#ifdef USE_AS_STRNCMP
	cmp	$1, %r11
	jbe	LABEL(ashr_14_exittail)
#endif

	pxor	%xmm0, %xmm0
	sub	$0x1000, %r10
	jmp	LABEL(gobble_ashr_14)

	.p2align 4
LABEL(ashr_14_exittail):
	movdqa	(%rsi, %rcx), %xmm1
	psrldq	$14, %xmm0
	psrldq	$14, %xmm3
	jmp	LABEL(aftertail)

/*
 *  The following cases will be handled by ashr_15
 *  rcx(offset of rsi)  rax(offset of rdi)        relative offset	 corresponding case
 *        n(1~15)          n - 1      	        14(15 +(n - 1) - n)         ashr_15
 */
	.p2align 4
LABEL(ashr_15):
	pxor	%xmm0, %xmm0
	movdqa	(%rdi), %xmm2
	movdqa	(%rsi), %xmm1
	pcmpeqb	%xmm1, %xmm0
	pslldq	$1, %xmm2
	pcmpeqb	%xmm1, %xmm2
	psubb	%xmm0, %xmm2
	pmovmskb %xmm2, %r9d
	shr	%cl, %edx
	shr	%cl, %r9d
	sub	%r9d, %edx
	jnz	LABEL(less32bytes)

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

	.p2align 4
LABEL(loop_ashr_15):
	add	$16, %r10
	jg	LABEL(nibble_ashr_15)

LABEL(gobble_ashr_15):
	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$15, %xmm3
	pslldq	$1, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$15, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3

	add	$16, %r10
	jg	LABEL(nibble_ashr_15)	/* cross page boundary */

	movdqa	(%rsi, %rcx), %xmm1
	movdqa	(%rdi, %rcx), %xmm2
	movdqa	%xmm2, %xmm4

#ifndef USE_SSSE3
	psrldq	$15, %xmm3
	pslldq	$1, %xmm2
	por	%xmm3, %xmm2		/* merge into one 16byte value */
#else
	palignr	$15, %xmm3, %xmm2	/* merge into one 16byte value */
#endif

	pcmpeqb	%xmm1, %xmm0
	pcmpeqb	%xmm2, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	sub	$0xffff, %edx
	jnz	LABEL(exit)

#ifdef USE_AS_STRNCMP
	sub	$16, %r11
	jbe	LABEL(strcmp_exitz)
#endif

	add	$16, %rcx
	movdqa	%xmm4, %xmm3
	jmp	LABEL(loop_ashr_15)

	.p2align 4
LABEL(nibble_ashr_15):
	pcmpeqb	%xmm3, %xmm0		/* check nibble for null char */
	pmovmskb %xmm0, %edx
	test	$0x8000, %edx
	jnz	LABEL(ashr_15_exittail)

#ifdef USE_AS_STRNCMP
	test	%r11, %r11
	je	LABEL(ashr_15_exittail)
#endif

	pxor	%xmm0, %xmm0
	sub	$0x1000, %r10
	jmp	LABEL(gobble_ashr_15)

	.p2align 4
LABEL(ashr_15_exittail):
	movdqa	(%rsi, %rcx), %xmm1
	psrldq	$15, %xmm3
	psrldq	$15, %xmm0

	.p2align 4
LABEL(aftertail):
	pcmpeqb	%xmm3, %xmm1
	psubb	%xmm0, %xmm1
	pmovmskb %xmm1, %edx
	not	%edx

	.p2align 4
LABEL(exit):
	lea	-16(%r9, %rcx), %rax	/* locate the exact offset for rdi */
LABEL(less32bytes):
	lea	(%rdi, %rax), %rdi	/* locate the exact address for first operand(rdi) */
	lea	(%rsi, %rcx), %rsi	/* locate the exact address for second operand(rsi) */
	test	%r8d, %r8d
	jz	LABEL(ret)
	xchg	%rsi, %rdi		/* recover original order according to flag(%r8d) */

	.p2align 4
LABEL(ret):
LABEL(less16bytes):
	bsf	%rdx, %rdx		/* find and store bit index in %rdx */

#ifdef USE_AS_STRNCMP
	sub	%rdx, %r11
	jbe	LABEL(strcmp_exitz)
#endif
	movzbl	(%rsi, %rdx), %ecx
	movzbl	(%rdi, %rdx), %eax

	sub	%ecx, %eax
	ret

LABEL(strcmp_exitz):
	xor	%eax, %eax
	ret

	.p2align 4
LABEL(Byte0):
	movzx	(%rsi), %ecx
	movzx	(%rdi), %eax

	sub	%ecx, %eax
	ret
END (BP_SYM (STRCMP))

	.section .rodata,"a",@progbits
	.p2align 3
LABEL(unaligned_table):
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
	.int	LABEL(ashr_0) - LABEL(unaligned_table)
#endif /* NOT_IN_libc */
libc_hidden_builtin_def (STRCMP)
