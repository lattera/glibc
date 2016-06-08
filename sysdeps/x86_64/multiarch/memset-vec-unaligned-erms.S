/* memset/bzero with unaligned store and rep stosb
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

/* memset is implemented as:
   1. Use overlapping store to avoid branch.
   2. If size is less than VEC, use integer register stores.
   3. If size is from VEC_SIZE to 2 * VEC_SIZE, use 2 VEC stores.
   4. If size is from 2 * VEC_SIZE to 4 * VEC_SIZE, use 4 VEC stores.
   5. If size is more to 4 * VEC_SIZE, align to 4 * VEC_SIZE with
      4 VEC stores and store 4 * VEC at a time until done.  */

#include <sysdep.h>

#ifndef MEMSET_CHK_SYMBOL
# define MEMSET_CHK_SYMBOL(p,s)		MEMSET_SYMBOL(p, s)
#endif

#ifndef VZEROUPPER
# if VEC_SIZE > 16
#  define VZEROUPPER			vzeroupper
# else
#  define VZEROUPPER
# endif
#endif

#ifndef VZEROUPPER_SHORT_RETURN
# if VEC_SIZE > 16
#  define VZEROUPPER_SHORT_RETURN	vzeroupper
# else
#  define VZEROUPPER_SHORT_RETURN	rep
# endif
#endif

#ifndef MOVQ
# if VEC_SIZE > 16
#  define MOVQ				vmovq
# else
#  define MOVQ				movq
# endif
#endif

/* Threshold to use Enhanced REP STOSB.  Since there is overhead to set
   up REP STOSB operation, REP STOSB isn't faster on short data.  The
   memset micro benchmark in glibc shows that 2KB is the approximate
   value above which REP STOSB becomes faster on processors with
   Enhanced REP STOSB.  Since the stored value is fixed, larger register
   size has minimal impact on threshold.  */
#ifndef REP_STOSB_THRESHOLD
# define REP_STOSB_THRESHOLD		2048
#endif

#ifndef SECTION
# error SECTION is not defined!
#endif

	.section SECTION(.text),"ax",@progbits
#if VEC_SIZE == 16 && IS_IN (libc)
ENTRY (__bzero)
	movq	%rdi, %rax /* Set return value.  */
	movq	%rsi, %rdx /* Set n.  */
	pxor	%xmm0, %xmm0
	jmp	L(entry_from_bzero)
END (__bzero)
weak_alias (__bzero, bzero)
#endif

#if defined SHARED && IS_IN (libc)
ENTRY_CHK (MEMSET_CHK_SYMBOL (__memset_chk, unaligned))
	cmpq	%rdx, %rcx
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END_CHK (MEMSET_CHK_SYMBOL (__memset_chk, unaligned))
#endif

ENTRY (MEMSET_SYMBOL (__memset, unaligned))
L(memset_entry):
	VDUP_TO_VEC0_AND_SET_RETURN (%esi, %rdi)
L(entry_from_bzero):
	cmpq	$VEC_SIZE, %rdx
	jb	L(less_vec)
	cmpq	$(VEC_SIZE * 2), %rdx
	ja	L(more_2x_vec)
	/* From VEC and to 2 * VEC.  No branch when size == VEC_SIZE.  */
	VMOVU	%VEC(0), -VEC_SIZE(%rdi,%rdx)
	VMOVU	%VEC(0), (%rdi)
	VZEROUPPER
	ret
#if defined USE_MULTIARCH && IS_IN (libc)
END (MEMSET_SYMBOL (__memset, unaligned))

# if VEC_SIZE == 16
/* Only used to measure performance of REP STOSB.  */
ENTRY (__memset_erms)
# else
/* Provide a symbol to debugger.  */
ENTRY (MEMSET_SYMBOL (__memset, erms))
# endif
L(stosb):
	movq	%rdx, %rcx
	movzbl	%sil, %eax
	movq	%rdi, %rdx
	rep stosb
	movq	%rdx, %rax
	ret
# if VEC_SIZE == 16
END (__memset_erms)
# else
END (MEMSET_SYMBOL (__memset, erms))
# endif

# if defined SHARED && IS_IN (libc)
ENTRY_CHK (MEMSET_CHK_SYMBOL (__memset_chk, unaligned_erms))
	cmpq	%rdx, %rcx
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END_CHK (MEMSET_CHK_SYMBOL (__memset_chk, unaligned_erms))
# endif

ENTRY (MEMSET_SYMBOL (__memset, unaligned_erms))
	VDUP_TO_VEC0_AND_SET_RETURN (%esi, %rdi)
	cmpq	$VEC_SIZE, %rdx
	jb	L(less_vec)
	cmpq	$(VEC_SIZE * 2), %rdx
	ja	L(stosb_more_2x_vec)
	/* From VEC and to 2 * VEC.  No branch when size == VEC_SIZE.  */
	VMOVU	%VEC(0), -VEC_SIZE(%rdi,%rdx)
	VMOVU	%VEC(0), (%rdi)
	VZEROUPPER
	ret

L(stosb_more_2x_vec):
	cmpq	$REP_STOSB_THRESHOLD, %rdx
	ja	L(stosb)
#endif
L(more_2x_vec):
	cmpq  $(VEC_SIZE * 4), %rdx
	ja	L(loop_start)
	VMOVU	%VEC(0), (%rdi)
	VMOVU	%VEC(0), VEC_SIZE(%rdi)
	VMOVU	%VEC(0), -VEC_SIZE(%rdi,%rdx)
	VMOVU	%VEC(0), -(VEC_SIZE * 2)(%rdi,%rdx)
L(return):
	VZEROUPPER
	ret

L(loop_start):
	leaq	(VEC_SIZE * 4)(%rdi), %rcx
	VMOVU	%VEC(0), (%rdi)
	andq	$-(VEC_SIZE * 4), %rcx
	VMOVU	%VEC(0), -VEC_SIZE(%rdi,%rdx)
	VMOVU	%VEC(0), VEC_SIZE(%rdi)
	VMOVU	%VEC(0), -(VEC_SIZE * 2)(%rdi,%rdx)
	VMOVU	%VEC(0), (VEC_SIZE * 2)(%rdi)
	VMOVU	%VEC(0), -(VEC_SIZE * 3)(%rdi,%rdx)
	VMOVU	%VEC(0), (VEC_SIZE * 3)(%rdi)
	VMOVU	%VEC(0), -(VEC_SIZE * 4)(%rdi,%rdx)
	addq	%rdi, %rdx
	andq	$-(VEC_SIZE * 4), %rdx
	cmpq	%rdx, %rcx
	je	L(return)
L(loop):
	VMOVA	%VEC(0), (%rcx)
	VMOVA	%VEC(0), VEC_SIZE(%rcx)
	VMOVA	%VEC(0), (VEC_SIZE * 2)(%rcx)
	VMOVA	%VEC(0), (VEC_SIZE * 3)(%rcx)
	addq	$(VEC_SIZE * 4), %rcx
	cmpq	%rcx, %rdx
	jne	L(loop)
	VZEROUPPER_SHORT_RETURN
	ret
L(less_vec):
	/* Less than 1 VEC.  */
# if VEC_SIZE != 16 && VEC_SIZE != 32 && VEC_SIZE != 64
#  error Unsupported VEC_SIZE!
# endif
# if VEC_SIZE > 32
	cmpb	$32, %dl
	jae	L(between_32_63)
# endif
# if VEC_SIZE > 16
	cmpb	$16, %dl
	jae	L(between_16_31)
# endif
	MOVQ	%xmm0, %rcx
	cmpb	$8, %dl
	jae	L(between_8_15)
	cmpb	$4, %dl
	jae	L(between_4_7)
	cmpb	$1, %dl
	ja	L(between_2_3)
	jb	1f
	movb	%cl, (%rdi)
1:
	VZEROUPPER
	ret
# if VEC_SIZE > 32
	/* From 32 to 63.  No branch when size == 32.  */
L(between_32_63):
	vmovdqu	%ymm0, -32(%rdi,%rdx)
	vmovdqu	%ymm0, (%rdi)
	VZEROUPPER
	ret
# endif
# if VEC_SIZE > 16
	/* From 16 to 31.  No branch when size == 16.  */
L(between_16_31):
	vmovdqu	%xmm0, -16(%rdi,%rdx)
	vmovdqu	%xmm0, (%rdi)
	VZEROUPPER
	ret
# endif
	/* From 8 to 15.  No branch when size == 8.  */
L(between_8_15):
	movq	%rcx, -8(%rdi,%rdx)
	movq	%rcx, (%rdi)
	VZEROUPPER
	ret
L(between_4_7):
	/* From 4 to 7.  No branch when size == 4.  */
	movl	%ecx, -4(%rdi,%rdx)
	movl	%ecx, (%rdi)
	VZEROUPPER
	ret
L(between_2_3):
	/* From 2 to 3.  No branch when size == 2.  */
	movw	%cx, -2(%rdi,%rdx)
	movw	%cx, (%rdi)
	VZEROUPPER
	ret
END (MEMSET_SYMBOL (__memset, unaligned_erms))
