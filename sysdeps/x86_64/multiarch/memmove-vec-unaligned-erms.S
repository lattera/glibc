/* memmove/memcpy/mempcpy with unaligned load/store and rep movsb
   Copyright (C) 2016 Free Software Foundation, Inc.
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

/* memmove/memcpy/mempcpy is implemented as:
   1. Use overlapping load and store to avoid branch.
   2. Use 8-bit or 32-bit displacements for branches and nop paddings
      to avoid long nop between instructions.
   3. Load all sources into registers and store them together to avoid
      possible address overflap between source and destination.
   4. If size is 2 * VEC_SIZE or less, load all sources into registers
      and store them together.
   5. If there is no address overflap, copy from both ends with
      4 * VEC_SIZE at a time.
   6. If size is 8 * VEC_SIZE or less, load all sources into registers
      and store them together.
   7. If address of destination > address of source, backward copy
      8 * VEC_SIZE at a time.
   8. Otherwise, forward copy 8 * VEC_SIZE at a time.  */

#include <sysdep.h>

#ifndef MEMCPY_SYMBOL
# define MEMCPY_SYMBOL(p,s)		MEMMOVE_SYMBOL(p, s)
#endif

#ifndef MEMPCPY_SYMBOL
# define MEMPCPY_SYMBOL(p,s)		MEMMOVE_SYMBOL(p, s)
#endif

#ifndef MEMMOVE_CHK_SYMBOL
# define MEMMOVE_CHK_SYMBOL(p,s)	MEMMOVE_SYMBOL(p, s)
#endif

#ifndef VZEROUPPER
# if VEC_SIZE > 16
#  define VZEROUPPER vzeroupper
# else
#  define VZEROUPPER
# endif
#endif

/* Threshold to use Enhanced REP MOVSB.  Since there is overhead to set
   up REP MOVSB operation, REP MOVSB isn't faster on short data.  The
   memcpy micro benchmark in glibc shows that 2KB is the approximate
   value above which REP MOVSB becomes faster than SSE2 optimization
   on processors with Enhanced REP MOVSB.  Since larger register size
   can move more data with a single load and store, the threshold is
   higher with larger register size.  */
#ifndef REP_MOVSB_THRESHOLD
# define REP_MOVSB_THRESHOLD	(2048 * (VEC_SIZE / 16))
#endif

#ifndef SECTION
# error SECTION is not defined!
#endif

	.section SECTION(.text),"ax",@progbits
#if defined SHARED && IS_IN (libc)
ENTRY (MEMMOVE_CHK_SYMBOL (__mempcpy_chk, unaligned_2))
	cmpq	%rdx, %rcx
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END (MEMMOVE_CHK_SYMBOL (__mempcpy_chk, unaligned_2))
#endif

#if VEC_SIZE == 16 || defined SHARED
ENTRY (MEMPCPY_SYMBOL (__mempcpy, unaligned_2))
	movq	%rdi, %rax
	addq	%rdx, %rax
	jmp	L(start)
END (MEMPCPY_SYMBOL (__mempcpy, unaligned_2))
#endif

#if defined SHARED && IS_IN (libc)
ENTRY (MEMMOVE_CHK_SYMBOL (__memmove_chk, unaligned_2))
	cmpq	%rdx, %rcx
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END (MEMMOVE_CHK_SYMBOL (__memmove_chk, unaligned_2))
#endif

ENTRY (MEMMOVE_SYMBOL (__memmove, unaligned_2))
	movq	%rdi, %rax
L(start):
	cmpq	$VEC_SIZE, %rdx
	jb	L(less_vec)
	cmpq	$(VEC_SIZE * 2), %rdx
	ja	L(more_2x_vec)
#if !defined USE_MULTIARCH || !IS_IN (libc)
L(last_2x_vec):
#endif
	/* From VEC and to 2 * VEC.  No branch when size == VEC_SIZE.  */
	VMOVU	(%rsi), %VEC(0)
	VMOVU	-VEC_SIZE(%rsi,%rdx), %VEC(1)
	VMOVU	%VEC(0), (%rdi)
	VMOVU	%VEC(1), -VEC_SIZE(%rdi,%rdx)
	VZEROUPPER
#if !defined USE_MULTIARCH || !IS_IN (libc)
L(nop):
#endif
	ret
#if defined USE_MULTIARCH && IS_IN (libc)
END (MEMMOVE_SYMBOL (__memmove, unaligned_2))

# if VEC_SIZE == 16 && defined SHARED
/* Only used to measure performance of REP MOVSB.  */
ENTRY (__mempcpy_erms)
	movq	%rdi, %rax
	addq	%rdx, %rax
	jmp	L(start_movsb)
END (__mempcpy_erms)

ENTRY (__memmove_erms)
	movq	%rdi, %rax
L(start_movsb):
	movq	%rdx, %rcx
	cmpq	%rsi, %rdi
	jb	1f
	/* Source == destination is less common.  */
	je	2f
	leaq	(%rsi,%rcx), %rdx
	cmpq	%rdx, %rdi
	jb	L(movsb_backward)
1:
	rep movsb
2:
	ret
L(movsb_backward):
	leaq	-1(%rdi,%rcx), %rdi
	leaq	-1(%rsi,%rcx), %rsi
	std
	rep movsb
	cld
	ret
END (__memmove_erms)
strong_alias (__memmove_erms, __memcpy_erms)
# endif

# ifdef SHARED
ENTRY (MEMMOVE_CHK_SYMBOL (__mempcpy_chk, unaligned_erms))
	cmpq	%rdx, %rcx
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END (MEMMOVE_CHK_SYMBOL (__mempcpy_chk, unaligned_erms))

ENTRY (MEMMOVE_SYMBOL (__mempcpy, unaligned_erms))
	movq	%rdi, %rax
	addq	%rdx, %rax
	jmp	L(start_erms)
END (MEMMOVE_SYMBOL (__mempcpy, unaligned_erms))

ENTRY (MEMMOVE_CHK_SYMBOL (__memmove_chk, unaligned_erms))
	cmpq	%rdx, %rcx
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END (MEMMOVE_CHK_SYMBOL (__memmove_chk, unaligned_erms))
# endif

ENTRY (MEMMOVE_SYMBOL (__memmove, unaligned_erms))
	movq	%rdi, %rax
L(start_erms):
	cmpq	$VEC_SIZE, %rdx
	jb	L(less_vec)
	cmpq	$(VEC_SIZE * 2), %rdx
	ja	L(movsb_more_2x_vec)
L(last_2x_vec):
	/* From VEC and to 2 * VEC.  No branch when size == VEC_SIZE. */
	VMOVU	(%rsi), %VEC(0)
	VMOVU	-VEC_SIZE(%rsi,%rdx), %VEC(1)
	VMOVU	%VEC(0), (%rdi)
	VMOVU	%VEC(1), -VEC_SIZE(%rdi,%rdx)
L(return):
	VZEROUPPER
	ret

L(movsb):
	cmpq	%rsi, %rdi
	jb	1f
	/* Source == destination is less common.  */
	je	L(nop)
	leaq	(%rsi,%rdx), %r9
	cmpq	%r9, %rdi
	/* Avoid slow backward REP MOVSB.  */
# if REP_MOVSB_THRESHOLD <= (VEC_SIZE * 8)
#  error Unsupported REP_MOVSB_THRESHOLD and VEC_SIZE!
# endif
	jb	L(more_8x_vec_backward)
1:
	movq	%rdx, %rcx
	rep movsb
L(nop):
	ret

	.p2align 4
L(movsb_more_2x_vec):
	cmpq	$REP_MOVSB_THRESHOLD, %rdx
	/* Force 32-bit displacement to avoid long nop between
	   instructions.  */
	ja.d32	L(movsb)
#endif
	.p2align 4
L(more_2x_vec):
	/* More than 2 * VEC.  */
	cmpq	%rsi, %rdi
	jb	L(copy_forward)
	/* Source == destination is less common.  */
	je	L(nop)
	leaq	(%rsi,%rdx), %rcx
	cmpq	%rcx, %rdi
	jb	L(more_2x_vec_overlap)
L(copy_forward):
	leaq	(%rdi,%rdx), %rcx
	cmpq	%rcx, %rsi
	jb	L(more_2x_vec_overlap)
	VMOVU	(%rsi), %VEC(0)
	VMOVU	VEC_SIZE(%rsi), %VEC(1)
	VMOVU	-VEC_SIZE(%rsi,%rdx), %VEC(2)
	VMOVU	-(VEC_SIZE * 2)(%rsi,%rdx), %VEC(3)
	VMOVU	%VEC(0), (%rdi)
	VMOVU	%VEC(1), VEC_SIZE(%rdi)
	VMOVU	%VEC(2), -VEC_SIZE(%rdi,%rdx)
	VMOVU	%VEC(3), -(VEC_SIZE * 2)(%rdi,%rdx)
	cmpq	$(VEC_SIZE * 4), %rdx
	/* Force 32-bit displacement to avoid long nop between
	   instructions.  */
	jbe.d32	L(return)
	VMOVU	(VEC_SIZE * 2)(%rsi), %VEC(0)
	VMOVU	(VEC_SIZE * 3)(%rsi), %VEC(1)
	VMOVU	-(VEC_SIZE * 3)(%rsi,%rdx), %VEC(2)
	VMOVU	-(VEC_SIZE * 4)(%rsi,%rdx), %VEC(3)
	VMOVU	%VEC(0), (VEC_SIZE * 2)(%rdi)
	VMOVU	%VEC(1), (VEC_SIZE * 3)(%rdi)
	VMOVU	%VEC(2), -(VEC_SIZE * 3)(%rdi,%rdx)
	VMOVU	%VEC(3), -(VEC_SIZE * 4)(%rdi,%rdx)
	cmpq	$(VEC_SIZE * 8), %rdx
#if  VEC_SIZE == 16
# if defined USE_MULTIARCH && IS_IN (libc)
	jbe	L(return)
# else
	/* Use 32-bit displacement to avoid long nop between
	   instructions.  */
	jbe.d32	L(return)
# endif
#else
	/* Use 8-bit displacement to avoid long nop between
	   instructions.  */
	jbe	L(return_disp8)
#endif
	leaq	(VEC_SIZE * 4)(%rdi), %rcx
	addq	%rdi, %rdx
	andq	$-(VEC_SIZE * 4), %rdx
	andq	$-(VEC_SIZE * 4), %rcx
	movq	%rcx, %r11
	subq	%rdi, %r11
	addq	%r11, %rsi
	cmpq	%rdx, %rcx
	/* Use 8-bit displacement to avoid long nop between
	   instructions.  */
	je	L(return_disp8)
	movq	%rsi, %r10
	subq	%rcx, %r10
	leaq	VEC_SIZE(%r10), %r9
	leaq	(VEC_SIZE * 2)(%r10), %r8
	leaq	(VEC_SIZE * 3)(%r10), %r11
	.p2align 4
L(loop):
	VMOVU	(%rcx,%r10), %VEC(0)
	VMOVU	(%rcx,%r9), %VEC(1)
	VMOVU	(%rcx,%r8), %VEC(2)
	VMOVU	(%rcx,%r11), %VEC(3)
	VMOVA	%VEC(0), (%rcx)
	VMOVA	%VEC(1), VEC_SIZE(%rcx)
	VMOVA	%VEC(2), (VEC_SIZE * 2)(%rcx)
	VMOVA	%VEC(3), (VEC_SIZE * 3)(%rcx)
	addq	$(VEC_SIZE * 4), %rcx
	cmpq	%rcx, %rdx
	jne	L(loop)
#if !defined USE_MULTIARCH || !IS_IN (libc)
L(return):
#endif
L(return_disp8):
	VZEROUPPER
	ret
L(less_vec):
	/* Less than 1 VEC.  */
#if VEC_SIZE != 16 && VEC_SIZE != 32 && VEC_SIZE != 64
# error Unsupported VEC_SIZE!
#endif
#if VEC_SIZE > 32
	cmpb	$32, %dl
	jae	L(between_32_63)
#endif
#if VEC_SIZE > 16
	cmpb	$16, %dl
	jae	L(between_16_31)
#endif
	cmpb	$8, %dl
	jae	L(between_8_15)
	cmpb	$4, %dl
	jae	L(between_4_7)
	cmpb	$1, %dl
	ja	L(between_2_3)
	jb	1f
	movzbl	(%rsi), %ecx
	movb	%cl, (%rdi)
1:
	ret
#if VEC_SIZE > 32
L(between_32_63):
	/* From 32 to 63.  No branch when size == 32.  */
	vmovdqu	(%rsi), %ymm0
	vmovdqu	-32(%rsi,%rdx), %ymm1
	vmovdqu	%ymm0, (%rdi)
	vmovdqu	%ymm1, -32(%rdi,%rdx)
	VZEROUPPER
	ret
#endif
#if VEC_SIZE > 16
	/* From 16 to 31.  No branch when size == 16.  */
L(between_16_31):
	vmovdqu	(%rsi), %xmm0
	vmovdqu	-16(%rsi,%rdx), %xmm1
	vmovdqu	%xmm0, (%rdi)
	vmovdqu	%xmm1, -16(%rdi,%rdx)
	ret
#endif
L(between_8_15):
	/* From 8 to 15.  No branch when size == 8.  */
	movq	-8(%rsi,%rdx), %rcx
	movq	(%rsi), %rsi
	movq	%rcx, -8(%rdi,%rdx)
	movq	%rsi, (%rdi)
	ret
L(between_4_7):
	/* From 4 to 7.  No branch when size == 4.  */
	movl	-4(%rsi,%rdx), %ecx
	movl	(%rsi), %esi
	movl	%ecx, -4(%rdi,%rdx)
	movl	%esi, (%rdi)
	ret
L(between_2_3):
	/* From 2 to 3.  No branch when size == 2.  */
	movzwl	-2(%rsi,%rdx), %ecx
	movzwl	(%rsi), %esi
	movw	%cx, -2(%rdi,%rdx)
	movw	%si, (%rdi)
	ret

#if VEC_SIZE > 16
	/* Align to 16 bytes to avoid long nop between instructions.  */
	.p2align 4
#endif
L(more_2x_vec_overlap):
	/* More than 2 * VEC and there is overlap bewteen destination
	   and source.  */
	cmpq	$(VEC_SIZE * 8), %rdx
	ja	L(more_8x_vec)
	cmpq	$(VEC_SIZE * 4), %rdx
	jb	L(last_4x_vec)
L(between_4x_vec_and_8x_vec):
	/* Copy from 4 * VEC to 8 * VEC, inclusively. */
	VMOVU	(%rsi), %VEC(0)
	VMOVU	VEC_SIZE(%rsi), %VEC(1)
	VMOVU	(VEC_SIZE * 2)(%rsi), %VEC(2)
	VMOVU	(VEC_SIZE * 3)(%rsi), %VEC(3)
	VMOVU	-VEC_SIZE(%rsi,%rdx), %VEC(4)
	VMOVU	-(VEC_SIZE * 2)(%rsi,%rdx), %VEC(5)
	VMOVU	-(VEC_SIZE * 3)(%rsi,%rdx), %VEC(6)
	VMOVU	-(VEC_SIZE * 4)(%rsi,%rdx), %VEC(7)
	VMOVU	%VEC(0), (%rdi)
	VMOVU	%VEC(1), VEC_SIZE(%rdi)
	VMOVU	%VEC(2), (VEC_SIZE * 2)(%rdi)
	VMOVU	%VEC(3), (VEC_SIZE * 3)(%rdi)
	VMOVU	%VEC(4), -VEC_SIZE(%rdi,%rdx)
	VMOVU	%VEC(5), -(VEC_SIZE * 2)(%rdi,%rdx)
	VMOVU	%VEC(6), -(VEC_SIZE * 3)(%rdi,%rdx)
	VMOVU	%VEC(7), -(VEC_SIZE * 4)(%rdi,%rdx)
	VZEROUPPER
	ret
L(last_4x_vec):
	/* Copy from 2 * VEC to 4 * VEC. */
	VMOVU	(%rsi), %VEC(0)
	VMOVU	VEC_SIZE(%rsi), %VEC(1)
	VMOVU	-VEC_SIZE(%rsi,%rdx), %VEC(2)
	VMOVU	-(VEC_SIZE * 2)(%rsi,%rdx), %VEC(3)
	VMOVU	%VEC(0), (%rdi)
	VMOVU	%VEC(1), VEC_SIZE(%rdi)
	VMOVU	%VEC(2), -VEC_SIZE(%rdi,%rdx)
	VMOVU	%VEC(3), -(VEC_SIZE * 2)(%rdi,%rdx)
	VZEROUPPER
	ret
L(between_0_and_4x_vec):
	/* Copy from 0 to 4 * VEC. */
	cmpl	$(VEC_SIZE * 2), %edx
	jae	L(last_4x_vec)
	/* Copy from 0 to 2 * VEC. */
	cmpl	$VEC_SIZE, %edx
	jae	L(last_2x_vec)
	/* Copy from 0 to VEC. */
	VZEROUPPER
	jmp	L(less_vec)
L(more_8x_vec):
	cmpq	%rsi, %rdi
	ja	L(more_8x_vec_backward)

	.p2align 4
L(loop_8x_vec_forward):
	/* Copy 8 * VEC a time forward.  */
	VMOVU	(%rsi), %VEC(0)
	VMOVU	VEC_SIZE(%rsi), %VEC(1)
	VMOVU	(VEC_SIZE * 2)(%rsi), %VEC(2)
	VMOVU	(VEC_SIZE * 3)(%rsi), %VEC(3)
	VMOVU	(VEC_SIZE * 4)(%rsi), %VEC(4)
	VMOVU	(VEC_SIZE * 5)(%rsi), %VEC(5)
	VMOVU	(VEC_SIZE * 6)(%rsi), %VEC(6)
	VMOVU	(VEC_SIZE * 7)(%rsi), %VEC(7)
	VMOVU	%VEC(0), (%rdi)
	VMOVU	%VEC(1), VEC_SIZE(%rdi)
	VMOVU	%VEC(2), (VEC_SIZE * 2)(%rdi)
	VMOVU	%VEC(3), (VEC_SIZE * 3)(%rdi)
	VMOVU	%VEC(4), (VEC_SIZE * 4)(%rdi)
	VMOVU	%VEC(5), (VEC_SIZE * 5)(%rdi)
	VMOVU	%VEC(6), (VEC_SIZE * 6)(%rdi)
	VMOVU	%VEC(7), (VEC_SIZE * 7)(%rdi)
	addq	$(VEC_SIZE * 8), %rdi
	addq	$(VEC_SIZE * 8), %rsi
	subq	$(VEC_SIZE * 8), %rdx
	cmpq	$(VEC_SIZE * 8), %rdx
	je	L(between_4x_vec_and_8x_vec)
	ja	L(loop_8x_vec_forward)
	/* Less than 8 * VEC to copy.  */
	cmpq	$(VEC_SIZE * 4), %rdx
	jb	L(between_0_and_4x_vec)
	jmp	L(between_4x_vec_and_8x_vec)

	.p2align 4
L(more_8x_vec_backward):
	leaq	-VEC_SIZE(%rsi, %rdx), %rcx
	leaq	-VEC_SIZE(%rdi, %rdx), %r9

	.p2align 4
L(loop_8x_vec_backward):
	/* Copy 8 * VEC a time backward.  */
	VMOVU	(%rcx), %VEC(0)
	VMOVU	-VEC_SIZE(%rcx), %VEC(1)
	VMOVU	-(VEC_SIZE * 2)(%rcx), %VEC(2)
	VMOVU	-(VEC_SIZE * 3)(%rcx), %VEC(3)
	VMOVU	-(VEC_SIZE * 4)(%rcx), %VEC(4)
	VMOVU	-(VEC_SIZE * 5)(%rcx), %VEC(5)
	VMOVU	-(VEC_SIZE * 6)(%rcx), %VEC(6)
	VMOVU	-(VEC_SIZE * 7)(%rcx), %VEC(7)
	VMOVU	%VEC(0), (%r9)
	VMOVU	%VEC(1), -VEC_SIZE(%r9)
	VMOVU	%VEC(2), -(VEC_SIZE * 2)(%r9)
	VMOVU	%VEC(3), -(VEC_SIZE * 3)(%r9)
	VMOVU	%VEC(4), -(VEC_SIZE * 4)(%r9)
	VMOVU	%VEC(5), -(VEC_SIZE * 5)(%r9)
	VMOVU	%VEC(6), -(VEC_SIZE * 6)(%r9)
	VMOVU	%VEC(7), -(VEC_SIZE * 7)(%r9)
	subq	$(VEC_SIZE * 8), %rcx
	subq	$(VEC_SIZE * 8), %r9
	subq	$(VEC_SIZE * 8), %rdx
	cmpq	$(VEC_SIZE * 8), %rdx
	je	L(between_4x_vec_and_8x_vec)
	ja	L(loop_8x_vec_backward)
	/* Less than 8 * VEC to copy.  */
	cmpq	$(VEC_SIZE * 4), %rdx
	jb	L(between_0_and_4x_vec)
	jmp	L(between_4x_vec_and_8x_vec)
END (MEMMOVE_SYMBOL (__memmove, unaligned_erms))

#ifdef SHARED
# if IS_IN (libc)
#  ifdef USE_MULTIARCH
strong_alias (MEMMOVE_SYMBOL (__memmove, unaligned_erms),
	      MEMMOVE_SYMBOL (__memcpy, unaligned_erms))
strong_alias (MEMMOVE_SYMBOL (__memmove_chk, unaligned_erms),
	      MEMMOVE_SYMBOL (__memcpy_chk, unaligned_erms))
#  endif
strong_alias (MEMMOVE_CHK_SYMBOL (__memmove_chk, unaligned_2),
	      MEMMOVE_CHK_SYMBOL (__memcpy_chk, unaligned_2))
# endif
#endif
#if VEC_SIZE == 16 || defined SHARED
strong_alias (MEMMOVE_SYMBOL (__memmove, unaligned_2),
	      MEMCPY_SYMBOL (__memcpy, unaligned_2))
#endif
