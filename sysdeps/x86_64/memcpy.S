/*
   Optimized memcpy for x86-64.

   Copyright (C) 2007 Free Software Foundation, Inc.
   Contributed by Evandro Menezes <evandro.menezes@amd.com>, 2007.

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
   <http://www.gnu.org/licenses/>.
*/

#include <sysdep.h>
#include "asm-syntax.h"

/* Stack slots in the red-zone. */

#ifdef USE_AS_MEMPCPY
#  define RETVAL	(0)
#else
#  define RETVAL	(-8)
#  if defined SHARED && !defined USE_MULTIARCH && !defined NOT_IN_libc
#    define memcpy	__memcpy
#    undef libc_hidden_builtin_def
#    define libc_hidden_builtin_def(name) \
	.globl __GI_memcpy; __GI_memcpy = __memcpy
#  endif
#endif
#define SAVE0	(RETVAL - 8)
#define SAVE1	(SAVE0	- 8)
#define SAVE2	(SAVE1	- 8)
#define SAVE3	(SAVE2	- 8)

        .text

#if defined PIC && !defined NOT_IN_libc
ENTRY_CHK (__memcpy_chk)

	cmpq	%rdx, %rcx
	jb	HIDDEN_JUMPTARGET (__chk_fail)

END_CHK (__memcpy_chk)
#endif

ENTRY(memcpy)				/* (void *, const void*, size_t) */

/* Handle tiny blocks. */

L(1try):				/* up to 32B */
	cmpq	$32, %rdx
#ifndef USE_AS_MEMPCPY
	movq	%rdi, %rax		/* save return value */
#endif
	jae	L(1after)

L(1):					/* 1-byte once */
	testb	$1, %dl
	jz	L(1a)

	movzbl	(%rsi),	%ecx
	movb	%cl, (%rdi)

	incq	%rsi
	incq	%rdi

	.p2align 4,, 4

L(1a):					/* 2-byte once */
	testb	$2, %dl
	jz	L(1b)

	movzwl	(%rsi),	%ecx
	movw	%cx, (%rdi)

	addq	$2, %rsi
	addq	$2, %rdi

	.p2align 4,, 4

L(1b):					/* 4-byte once */
	testb	$4, %dl
	jz	L(1c)

	movl	(%rsi),	%ecx
	movl	%ecx, (%rdi)

	addq	$4, %rsi
	addq	$4, %rdi

	.p2align 4,, 4

L(1c):					/* 8-byte once */
	testb	$8, %dl
	jz	L(1d)

	movq	(%rsi), %rcx
	movq	%rcx, (%rdi)

	addq	$8, %rsi
	addq	$8, %rdi

	.p2align 4,, 4

L(1d):					/* 16-byte loop */
	andl	$0xf0, %edx
	jz	L(exit)

	.p2align 4

L(1loop):
	movq	 (%rsi), %rcx
	movq	8(%rsi), %r8
	movq	%rcx,  (%rdi)
	movq	 %r8, 8(%rdi)

	subl	$16, %edx

	leaq	16(%rsi), %rsi
	leaq	16(%rdi), %rdi

	jnz	L(1loop)

	.p2align 4,, 4

L(exit):				/* exit */
#ifdef USE_AS_MEMPCPY
	movq	%rdi, %rax		/* return value */
#else
	rep
#endif
	retq

	.p2align 4

L(1after):
#ifndef USE_AS_MEMPCPY
	movq	%rax, RETVAL(%rsp)	/* save return value */
#endif

/* Align to the natural word size. */

L(aligntry):
	movl	%esi, %ecx      	/* align by source */

	andl	$7, %ecx
	jz	L(alignafter)  		/* already aligned */

L(align):		      		/* align */
	leaq	-8(%rcx, %rdx), %rdx	/* calculate remaining bytes */
	subl	$8, %ecx

	.p2align 4

L(alignloop):				/* 1-byte alignment loop */
	movzbl	(%rsi), %eax
	movb	%al, (%rdi)

	incl	%ecx

	leaq	1(%rsi), %rsi
	leaq	1(%rdi), %rdi

	jnz	L(alignloop)

	.p2align 4

L(alignafter):

/* Handle mid-sized blocks. */

L(32try):				/* up to 1KB */
	cmpq	$1024, %rdx
	ja	L(32after)

L(32):					/* 32-byte loop */
	movl	%edx, %ecx
	shrl	$5, %ecx
	jz	L(32skip)

	.p2align 4

L(32loop):
	decl	%ecx

	movq	  (%rsi), %rax
	movq	 8(%rsi), %r8
	movq	16(%rsi), %r9
	movq	24(%rsi), %r10

	movq	%rax,   (%rdi)
	movq	 %r8,  8(%rdi)
	movq	 %r9, 16(%rdi)
	movq	%r10, 24(%rdi)

	leaq	32(%rsi), %rsi
	leaq	32(%rdi), %rdi

	jz	L(32skip)		/* help out smaller blocks */

	decl	%ecx

	movq	  (%rsi), %rax
	movq	 8(%rsi), %r8
	movq	16(%rsi), %r9
	movq	24(%rsi), %r10

	movq	%rax,   (%rdi)
	movq	 %r8,  8(%rdi)
	movq	 %r9, 16(%rdi)
	movq	%r10, 24(%rdi)

	leaq	32(%rsi), %rsi
	leaq	32(%rdi), %rdi

	jnz	L(32loop)

	.p2align 4

L(32skip):
	andl	$31, %edx		/* check for left overs */
#ifdef USE_AS_MEMPCPY
	jnz	L(1)

	movq	%rdi, %rax
#else
	movq	RETVAL(%rsp), %rax
	jnz	L(1)

	rep
#endif
	retq				/* exit */

	.p2align 4

L(32after):

/*
	In order to minimize code-size in RTLD, algorithms specific for
	larger blocks are excluded when building for RTLD.
*/

/* Handle blocks smaller than 1/2 L1. */

L(fasttry):				/* first 1/2 L1 */
#ifndef NOT_IN_libc			/* only up to this algorithm outside of libc.so */
	mov	__x86_64_data_cache_size_half(%rip), %R11_LP
	cmpq	%rdx, %r11		/* calculate the smaller of */
	cmovaq	%rdx, %r11		/* remaining bytes and 1/2 L1 */
#endif

L(fast):				/* good ol' MOVS */
#ifndef NOT_IN_libc
	movq	%r11, %rcx
	andq	$-8, %r11
#else
	movq	%rdx, %rcx
#endif
	shrq	$3, %rcx
	jz	L(fastskip)

	rep
	movsq

	.p2align 4,, 4

L(fastskip):
#ifndef NOT_IN_libc
	subq	%r11, %rdx		/* check for more */
	testq	$-8, %rdx
	jnz	L(fastafter)
#endif

	andl	$7, %edx		/* check for left overs */
#ifdef USE_AS_MEMPCPY
	jnz	L(1)

	movq	%rdi, %rax
#else
	movq	RETVAL(%rsp), %rax
	jnz	L(1)

	rep
#endif
	retq				/* exit */

#ifndef NOT_IN_libc			/* none of the algorithms below for RTLD */

	.p2align 4

L(fastafter):

/* Handle large blocks smaller than 1/2 L2. */

L(pretry):				/* first 1/2 L2 */
	mov	__x86_64_shared_cache_size_half (%rip), %R8_LP
	cmpq	%rdx, %r8		/* calculate the lesser of */
	cmovaq	%rdx, %r8		/* remaining bytes and 1/2 L2 */

L(pre):					/* 64-byte with prefetching */
	movq	%r8, %rcx
	andq	$-64, %r8
	shrq	$6, %rcx
	jz	L(preskip)

	movq	%r14, SAVE0(%rsp)
	cfi_rel_offset (%r14, SAVE0)
	movq	%r13, SAVE1(%rsp)
	cfi_rel_offset (%r13, SAVE1)
	movq	%r12, SAVE2(%rsp)
	cfi_rel_offset (%r12, SAVE2)
	movq	%rbx, SAVE3(%rsp)
	cfi_rel_offset (%rbx, SAVE3)

	cmpl	$0, __x86_64_prefetchw(%rip)
	jz	L(preloop)		/* check if PREFETCHW OK */

	.p2align 4

/* ... when PREFETCHW is available (less cache-probe traffic in MP systems). */

L(prewloop):				/* cache-line in state M */
	decq	%rcx

	movq	   (%rsi), %rax
	movq	 8 (%rsi), %rbx
	movq	16 (%rsi), %r9
	movq	24 (%rsi), %r10
	movq	32 (%rsi), %r11
	movq	40 (%rsi), %r12
	movq	48 (%rsi), %r13
	movq	56 (%rsi), %r14

	prefetcht0	 0 + 896 (%rsi)
	prefetcht0	64 + 896 (%rsi)

	movq	%rax,   (%rdi)
	movq	%rbx,  8(%rdi)
	movq	 %r9, 16(%rdi)
	movq	%r10, 24(%rdi)
	movq	%r11, 32(%rdi)
	movq	%r12, 40(%rdi)
	movq	%r13, 48(%rdi)
	movq	%r14, 56(%rdi)

	leaq	64(%rsi), %rsi
	leaq	64(%rdi), %rdi

	jz	L(prebail)

	decq	%rcx

	movq	  (%rsi), %rax
	movq	 8(%rsi), %rbx
	movq	16(%rsi), %r9
	movq	24(%rsi), %r10
	movq	32(%rsi), %r11
	movq	40(%rsi), %r12
	movq	48(%rsi), %r13
	movq	56(%rsi), %r14

	movq	%rax,   (%rdi)
	movq	%rbx,  8(%rdi)
	movq	 %r9, 16(%rdi)
	movq	%r10, 24(%rdi)
	movq	%r11, 32(%rdi)
	movq	%r12, 40(%rdi)
	movq	%r13, 48(%rdi)
	movq	%r14, 56(%rdi)

	prefetchw	896 - 64(%rdi)
	prefetchw	896 -  0(%rdi)

	leaq	64(%rsi), %rsi
	leaq	64(%rdi), %rdi

	jnz	L(prewloop)
	jmp	L(prebail)

	.p2align 4

/* ... when PREFETCHW is not available. */

L(preloop):				/* cache-line in state E */
	decq	%rcx

	movq	  (%rsi), %rax
	movq	 8(%rsi), %rbx
	movq	16(%rsi), %r9
	movq	24(%rsi), %r10
	movq	32(%rsi), %r11
	movq	40(%rsi), %r12
	movq	48(%rsi), %r13
	movq	56(%rsi), %r14

	prefetcht0	896 +  0(%rsi)
	prefetcht0	896 + 64(%rsi)

	movq	%rax,   (%rdi)
	movq	%rbx,  8(%rdi)
	movq	 %r9, 16(%rdi)
	movq	%r10, 24(%rdi)
	movq	%r11, 32(%rdi)
	movq	%r12, 40(%rdi)
	movq	%r13, 48(%rdi)
	movq	%r14, 56(%rdi)

	leaq	64 (%rsi), %rsi
	leaq	64 (%rdi), %rdi

	jz	L(prebail)

	decq	%rcx

	movq	  (%rsi), %rax
	movq	 8(%rsi), %rbx
	movq	16(%rsi), %r9
	movq	24(%rsi), %r10
	movq	32(%rsi), %r11
	movq	40(%rsi), %r12
	movq	48(%rsi), %r13
	movq	56(%rsi), %r14

	prefetcht0	896 - 64(%rdi)
	prefetcht0	896 -  0(%rdi)

	movq	%rax,   (%rdi)
	movq	%rbx,  8(%rdi)
	movq	 %r9, 16(%rdi)
	movq	%r10, 24(%rdi)
	movq	%r11, 32(%rdi)
	movq	%r12, 40(%rdi)
	movq	%r13, 48(%rdi)
	movq	%r14, 56(%rdi)

	leaq	64(%rsi), %rsi
	leaq	64(%rdi), %rdi

	jnz	L(preloop)

L(prebail):
	movq	SAVE3(%rsp), %rbx
	cfi_restore (%rbx)
	movq	SAVE2(%rsp), %r12
	cfi_restore (%r12)
	movq	SAVE1(%rsp), %r13
	cfi_restore (%r13)
	movq	SAVE0(%rsp), %r14
	cfi_restore (%r14)

/*       .p2align 4 */

L(preskip):
	subq	%r8, %rdx		/* check for more */
	testq	$-64, %rdx
	jnz	L(preafter)

	andl	$63, %edx		/* check for left overs */
#ifdef USE_AS_MEMPCPY
	jnz	L(1)

	movq	%rdi, %rax
#else
	movq	RETVAL(%rsp), %rax
	jnz	L(1)

	rep
#endif
	retq				/* exit */

	.p2align 4

L(preafter):

/* Handle huge blocks. */

L(NTtry):

L(NT):					/* non-temporal 128-byte */
	movq	%rdx, %rcx
	shrq	$7, %rcx
	jz	L(NTskip)

	movq	%r14, SAVE0(%rsp)
	cfi_rel_offset (%r14, SAVE0)
	movq	%r13, SAVE1(%rsp)
	cfi_rel_offset (%r13, SAVE1)
	movq	%r12, SAVE2(%rsp)
	cfi_rel_offset (%r12, SAVE2)

       .p2align 4

L(NTloop):
	prefetchnta	768(%rsi)
	prefetchnta	832(%rsi)

	decq	%rcx

	movq	  (%rsi), %rax
	movq	 8(%rsi), %r8
	movq	16(%rsi), %r9
	movq	24(%rsi), %r10
	movq	32(%rsi), %r11
	movq	40(%rsi), %r12
	movq	48(%rsi), %r13
	movq	56(%rsi), %r14

	movntiq	%rax,   (%rdi)
	movntiq	 %r8,  8(%rdi)
	movntiq	 %r9, 16(%rdi)
	movntiq	%r10, 24(%rdi)
	movntiq	%r11, 32(%rdi)
	movntiq	%r12, 40(%rdi)
	movntiq	%r13, 48(%rdi)
	movntiq	%r14, 56(%rdi)

	movq	 64(%rsi), %rax
	movq	 72(%rsi), %r8
	movq	 80(%rsi), %r9
	movq	 88(%rsi), %r10
	movq	 96(%rsi), %r11
	movq	104(%rsi), %r12
	movq	112(%rsi), %r13
	movq	120(%rsi), %r14

	movntiq	%rax,  64(%rdi)
	movntiq	 %r8,  72(%rdi)
	movntiq	 %r9,  80(%rdi)
	movntiq	%r10,  88(%rdi)
	movntiq	%r11,  96(%rdi)
	movntiq	%r12, 104(%rdi)
	movntiq	%r13, 112(%rdi)
	movntiq	%r14, 120(%rdi)

	leaq	128(%rsi), %rsi
	leaq	128(%rdi), %rdi

	jnz	L(NTloop)

	sfence				/* serialize memory stores */

	movq	SAVE2(%rsp), %r12
	cfi_restore (%r12)
	movq	SAVE1(%rsp), %r13
	cfi_restore (%r13)
	movq	SAVE0(%rsp), %r14
	cfi_restore (%r14)

L(NTskip):
	andl	$127, %edx		/* check for left overs */
#ifdef USE_AS_MEMPCPY
	jnz	L(1)

	movq	%rdi, %rax
#else
	movq	RETVAL(%rsp), %rax
	jnz	L(1)

	rep
#endif
	retq				/* exit */

#endif /* !NOT_IN_libc */

END(memcpy)

#ifndef USE_AS_MEMPCPY
libc_hidden_builtin_def (memcpy)
# if defined SHARED && !defined USE_MULTIARCH && !defined NOT_IN_libc
#  undef memcpy
#  include <shlib-compat.h>
versioned_symbol (libc, __memcpy, memcpy, GLIBC_2_14);
# endif
#endif
