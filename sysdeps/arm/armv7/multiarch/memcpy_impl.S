/* NEON/VFP/ARM version of memcpy optimized for Cortex-A15.
   Copyright (C) 2013-2016 Free Software Foundation, Inc.
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

   This memcpy routine is optimised for Cortex-A15 cores and takes advantage
   of VFP or NEON when built with the appropriate flags.

   Assumptions:

    ARMv6 (ARMv7-a if using Neon)
    ARM state
    Unaligned accesses

 */

/* Thumb cannot encode negative immediate offsets in memory operations.  */
#ifndef NO_THUMB
#define NO_THUMB
#endif
#include <sysdep.h>
#include <arm-features.h>

	.syntax unified
	/* This implementation requires ARM state.  */
	.arm

#ifdef MEMCPY_NEON

	.fpu	neon
	.arch	armv7-a
# define FRAME_SIZE	4
# define USE_VFP
# define USE_NEON

#elif defined (MEMCPY_VFP)

	.arch	armv6
	.fpu	vfpv2
# define FRAME_SIZE	32
# define USE_VFP

#else
	.arch	armv6
# define FRAME_SIZE    32

#endif

#define ALIGN(addr, align) addr:align

#define INSN_SIZE	4

/* Call parameters.  */
#define dstin	r0
#define src	r1
#define count	r2

/* Locals.  */
#define tmp1	r3
#define dst	ip
#define tmp2	r8

/* These two macros both work by repeated invocation of the macro
   dispatch_step (not defined here).  That macro performs one "step",
   doing one load instruction and one store instruction to copy one
   "unit".  On entry, TMP1 contains the number of bytes to be copied,
   a multiple of the unit size.  The macro clobbers TMP1 in the
   process of doing a computed jump to the tail containing the
   appropriate number of steps.

   In dispatch_7_dword, dispatch_step is invoked seven times, with an
   argument that is 7 for the first and 1 for the last.  Units are
   double-words (8 bytes).  TMP1 is at most 56.

   In dispatch_15_word, dispatch_step is invoked fifteen times,
   with an argument that is 15 for the first and 1 for the last.
   Units are words (4 bytes).  TMP1 is at most 60.  */

#ifndef ARM_ALWAYS_BX
# if ARM_BX_ALIGN_LOG2 != 2
#  error case not handled
# endif
	.macro dispatch_7_dword
	rsb	tmp1, tmp1, #((7 * 8) - PC_OFS + INSN_SIZE)
	add	pc, pc, tmp1
	dispatch_step 7
	dispatch_step 6
	dispatch_step 5
	dispatch_step 4
	dispatch_step 3
	dispatch_step 2
	dispatch_step 1
	.purgem dispatch_step
	.endm

	.macro dispatch_15_word
	rsb	tmp1, tmp1, #((15 * 4) - PC_OFS/2 + INSN_SIZE/2)
	add	pc, pc, tmp1, lsl #1
	dispatch_step 15
	dispatch_step 14
	dispatch_step 13
	dispatch_step 12
	dispatch_step 11
	dispatch_step 10
	dispatch_step 9
	dispatch_step 8
	dispatch_step 7
	dispatch_step 6
	dispatch_step 5
	dispatch_step 4
	dispatch_step 3
	dispatch_step 2
	dispatch_step 1
	.purgem dispatch_step
	.endm
#else
# if ARM_BX_ALIGN_LOG2 < 3
#  error case not handled
# endif
	.macro dispatch_helper steps, log2_bytes_per_step
	/* TMP1 gets (max_bytes - bytes_to_copy), where max_bytes is
	   (STEPS << LOG2_BYTES_PER_STEP).
	   So this is (steps_to_skip << LOG2_BYTES_PER_STEP).
	   Then it needs further adjustment to compensate for the
	   distance between the PC value taken below (0f + PC_OFS)
	   and the first step's instructions (1f).  */
	rsb	tmp1, tmp1, #((\steps << \log2_bytes_per_step) \
			      + ((1f - PC_OFS - 0f) \
				 >> (ARM_BX_ALIGN_LOG2 - \log2_bytes_per_step)))
	/* Shifting down LOG2_BYTES_PER_STEP gives us the number of
	   steps to skip, then shifting up ARM_BX_ALIGN_LOG2 gives us
	   the (byte) distance to add to the PC.  */
0:	add	tmp1, pc, tmp1, lsl #(ARM_BX_ALIGN_LOG2 - \log2_bytes_per_step)
	bx	tmp1
	.p2align ARM_BX_ALIGN_LOG2
1:
	.endm

	.macro dispatch_7_dword
	dispatch_helper 7, 3
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 7
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 6
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 5
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 4
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 3
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 2
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 1
	.p2align ARM_BX_ALIGN_LOG2
	.purgem dispatch_step
	.endm

	.macro dispatch_15_word
	dispatch_helper 15, 2
	dispatch_step 15
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 14
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 13
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 12
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 11
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 10
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 9
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 8
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 7
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 6
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 5
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 4
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 3
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 2
	.p2align ARM_BX_ALIGN_LOG2
	dispatch_step 1
	.p2align ARM_BX_ALIGN_LOG2
	.purgem dispatch_step
	.endm

#endif

#ifndef USE_NEON
/* For bulk copies using GP registers.  */
#define	A_l	r2		/* Call-clobbered.  */
#define	A_h	r3		/* Call-clobbered.  */
#define	B_l	r4
#define	B_h	r5
#define	C_l	r6
#define	C_h	r7
/* Don't use the pair r8,r9 because in some EABI variants r9 is reserved.  */
#define	D_l	r10
#define	D_h	r11
#endif

/* Number of lines ahead to pre-fetch data.  If you change this the code
   below will need adjustment to compensate.  */

#define prefetch_lines	5

#ifdef USE_VFP
	.macro	cpy_line_vfp vreg, base
	sfi_breg dst, \
	vstr	\vreg, [\B, #\base]
	sfi_breg src, \
	vldr	\vreg, [\B, #\base]
	sfi_breg dst, \
	vstr	d0, [\B, #\base + 8]
	sfi_breg src, \
	vldr	d0, [\B, #\base + 8]
	sfi_breg dst, \
	vstr	d1, [\B, #\base + 16]
	sfi_breg src, \
	vldr	d1, [\B, #\base + 16]
	sfi_breg dst, \
	vstr	d2, [\B, #\base + 24]
	sfi_breg src, \
	vldr	d2, [\B, #\base + 24]
	sfi_breg dst, \
	vstr	\vreg, [\B, #\base + 32]
	sfi_breg src, \
	vldr	\vreg, [\B, #\base + prefetch_lines * 64 - 32]
	sfi_breg dst, \
	vstr	d0, [\B, #\base + 40]
	sfi_breg src, \
	vldr	d0, [\B, #\base + 40]
	sfi_breg dst, \
	vstr	d1, [\B, #\base + 48]
	sfi_breg src, \
	vldr	d1, [\B, #\base + 48]
	sfi_breg dst, \
	vstr	d2, [\B, #\base + 56]
	sfi_breg src, \
	vldr	d2, [\B, #\base + 56]
	.endm

	.macro	cpy_tail_vfp vreg, base
	sfi_breg dst, \
	vstr	\vreg, [\B, #\base]
	sfi_breg src, \
	vldr	\vreg, [\B, #\base]
	sfi_breg dst, \
	vstr	d0, [\B, #\base + 8]
	sfi_breg src, \
	vldr	d0, [\B, #\base + 8]
	sfi_breg dst, \
	vstr	d1, [\B, #\base + 16]
	sfi_breg src, \
	vldr	d1, [\B, #\base + 16]
	sfi_breg dst, \
	vstr	d2, [\B, #\base + 24]
	sfi_breg src, \
	vldr	d2, [\B, #\base + 24]
	sfi_breg dst, \
	vstr	\vreg, [\B, #\base + 32]
	sfi_breg dst, \
	vstr	d0, [\B, #\base + 40]
	sfi_breg src, \
	vldr	d0, [\B, #\base + 40]
	sfi_breg dst, \
	vstr	d1, [\B, #\base + 48]
	sfi_breg src, \
	vldr	d1, [\B, #\base + 48]
	sfi_breg dst, \
	vstr	d2, [\B, #\base + 56]
	sfi_breg src, \
	vldr	d2, [\B, #\base + 56]
	.endm
#endif

	.p2align 6
ENTRY(memcpy)

	mov	dst, dstin	/* Preserve dstin, we need to return it.  */
	cmp	count, #64
	bge	.Lcpy_not_short
	/* Deal with small copies quickly by dropping straight into the
	   exit block.  */

.Ltail63unaligned:
#ifdef USE_NEON
	/* These need an extra layer of macro just to work around a
	   bug in the assembler's parser when an operand starts with
	   a {...}.  http://sourceware.org/bugzilla/show_bug.cgi?id=15647
	   tracks that bug; it was not fixed as of binutils-2.23.2.  */
	.macro neon_load_d0 reg
	vld1.8	{d0}, [\reg]!
	.endm
	.macro neon_store_d0 reg
	vst1.8	{d0}, [\reg]!
	.endm

	/* These are used by the NaCl sfi_breg macro.  */
	.macro _sfi_breg_dmask_neon_load_d0 reg
	_sfi_dmask \reg
	.endm
	.macro _sfi_breg_dmask_neon_store_d0 reg
	_sfi_dmask \reg
	.endm

	and	tmp1, count, #0x38
	.macro dispatch_step i
	sfi_breg src, neon_load_d0 \B
	sfi_breg dst, neon_store_d0 \B
	.endm
	dispatch_7_dword

	tst	count, #4
	sfi_breg src, \
	ldrne	tmp1, [\B], #4
	sfi_breg dst, \
	strne	tmp1, [\B], #4
#else
	/* Copy up to 15 full words of data.  May not be aligned.  */
	/* Cannot use VFP for unaligned data.  */
	and	tmp1, count, #0x3c
	add	dst, dst, tmp1
	add	src, src, tmp1
	/* Jump directly into the sequence below at the correct offset.  */
	.macro dispatch_step i
	sfi_breg src, \
	ldr	tmp1, [\B, #-(\i * 4)]
	sfi_breg dst, \
	str	tmp1, [\B, #-(\i * 4)]
	.endm
	dispatch_15_word
#endif

	lsls	count, count, #31
	sfi_breg src, \
	ldrhcs	tmp1, [\B], #2
	sfi_breg src, \
	ldrbne	src, [\B]		/* Src is dead, use as a scratch.  */
	sfi_breg dst, \
	strhcs	tmp1, [\B], #2
	sfi_breg dst, \
	strbne	src, [\B]
	bx	lr

.Lcpy_not_short:
	/* At least 64 bytes to copy, but don't know the alignment yet.  */
	str	tmp2, [sp, #-FRAME_SIZE]!
	cfi_adjust_cfa_offset (FRAME_SIZE)
	cfi_rel_offset (tmp2, 0)
	cfi_remember_state
	and	tmp2, src, #7
	and	tmp1, dst, #7
	cmp	tmp1, tmp2
	bne	.Lcpy_notaligned

#ifdef USE_VFP
	/* Magic dust alert!  Force VFP on Cortex-A9.  Experiments show
	   that the FP pipeline is much better at streaming loads and
	   stores.  This is outside the critical loop.  */
	vmov.f32	s0, s0
#endif

	/* SRC and DST have the same mutual 64-bit alignment, but we may
	   still need to pre-copy some bytes to get to natural alignment.
	   We bring SRC and DST into full 64-bit alignment.  */
	lsls	tmp2, dst, #29
	beq	1f
	rsbs	tmp2, tmp2, #0
	sub	count, count, tmp2, lsr #29
	sfi_breg src, \
	ldrmi	tmp1, [\B], #4
	sfi_breg dst, \
	strmi	tmp1, [\B], #4
	lsls	tmp2, tmp2, #2
	sfi_breg src, \
	ldrhcs	tmp1, [\B], #2
	sfi_breg src, \
	ldrbne	tmp2, [\B], #1
	sfi_breg dst, \
	strhcs	tmp1, [\B], #2
	sfi_breg dst, \
	strbne	tmp2, [\B], #1

1:
	subs	tmp2, count, #64	/* Use tmp2 for count.  */
	blt	.Ltail63aligned

	cmp	tmp2, #512
	bge	.Lcpy_body_long

.Lcpy_body_medium:			/* Count in tmp2.  */
#ifdef USE_VFP
1:
	sfi_breg src, \
	vldr	d0, [\B, #0]
	subs	tmp2, tmp2, #64
	sfi_breg src, \
	vldr	d1, [\B, #8]
	sfi_breg dst, \
	vstr	d0, [\B, #0]
	sfi_breg src, \
	vldr	d0, [\B, #16]
	sfi_breg dst, \
	vstr	d1, [\B, #8]
	sfi_breg src, \
	vldr	d1, [\B, #24]
	sfi_breg dst, \
	vstr	d0, [\B, #16]
	sfi_breg src, \
	vldr	d0, [\B, #32]
	sfi_breg dst, \
	vstr	d1, [\B, #24]
	sfi_breg src, \
	vldr	d1, [\B, #40]
	sfi_breg dst, \
	vstr	d0, [\B, #32]
	sfi_breg src, \
	vldr	d0, [\B, #48]
	sfi_breg dst, \
	vstr	d1, [\B, #40]
	sfi_breg src, \
	vldr	d1, [\B, #56]
	sfi_breg dst, \
	vstr	d0, [\B, #48]
	add	src, src, #64
	sfi_breg dst, \
	vstr	d1, [\B, #56]
	add	dst, dst, #64
	bge	1b
	tst	tmp2, #0x3f
	beq	.Ldone

.Ltail63aligned:			/* Count in tmp2.  */
	and	tmp1, tmp2, #0x38
	add	dst, dst, tmp1
	add	src, src, tmp1
	.macro dispatch_step i
	sfi_breg src, \
	vldr	d0, [\B, #-(\i * 8)]
	sfi_breg dst, \
	vstr	d0, [\B, #-(\i * 8)]
	.endm
	dispatch_7_dword
#else
	sub	src, src, #8
	sub	dst, dst, #8
1:
	sfi_breg src, \
	ldrd	A_l, A_h, [\B, #8]
	sfi_breg dst, \
	strd	A_l, A_h, [\B, #8]
	sfi_breg src, \
	ldrd	A_l, A_h, [\B, #16]
	sfi_breg dst, \
	strd	A_l, A_h, [\B, #16]
	sfi_breg src, \
	ldrd	A_l, A_h, [\B, #24]
	sfi_breg dst, \
	strd	A_l, A_h, [\B, #24]
	sfi_breg src, \
	ldrd	A_l, A_h, [\B, #32]
	sfi_breg dst, \
	strd	A_l, A_h, [\B, #32]
	sfi_breg src, \
	ldrd	A_l, A_h, [\B, #40]
	sfi_breg dst, \
	strd	A_l, A_h, [\B, #40]
	sfi_breg src, \
	ldrd	A_l, A_h, [\B, #48]
	sfi_breg dst, \
	strd	A_l, A_h, [\B, #48]
	sfi_breg src, \
	ldrd	A_l, A_h, [\B, #56]
	sfi_breg dst, \
	strd	A_l, A_h, [\B, #56]
	sfi_breg src, \
	ldrd	A_l, A_h, [\B, #64]!
	sfi_breg dst, \
	strd	A_l, A_h, [\B, #64]!
	subs	tmp2, tmp2, #64
	bge	1b
	tst	tmp2, #0x3f
	bne	1f
	ldr	tmp2,[sp], #FRAME_SIZE
	cfi_adjust_cfa_offset (-FRAME_SIZE)
	cfi_restore (tmp2)
	bx	lr

	cfi_restore_state
	cfi_remember_state
1:
	add	src, src, #8
	add	dst, dst, #8

.Ltail63aligned:			/* Count in tmp2.  */
	/* Copy up to 7 d-words of data.  Similar to Ltail63unaligned, but
	   we know that the src and dest are 64-bit aligned so we can use
	   LDRD/STRD to improve efficiency.  */
	/* TMP2 is now negative, but we don't care about that.  The bottom
	   six bits still tell us how many bytes are left to copy.  */

	and	tmp1, tmp2, #0x38
	add	dst, dst, tmp1
	add	src, src, tmp1
	.macro dispatch_step i
	sfi_breg src, \
	ldrd	A_l, A_h, [\B, #-(\i * 8)]
	sfi_breg dst, \
	strd	A_l, A_h, [\B, #-(\i * 8)]
	.endm
	dispatch_7_dword
#endif

	tst	tmp2, #4
	sfi_breg src, \
	ldrne	tmp1, [\B], #4
	sfi_breg dst, \
	strne	tmp1, [\B], #4
	lsls	tmp2, tmp2, #31		/* Count (tmp2) now dead. */
	sfi_breg src, \
	ldrhcs	tmp1, [\B], #2
	sfi_breg src, \
	ldrbne	tmp2, [\B]
	sfi_breg dst, \
	strhcs	tmp1, [\B], #2
	sfi_breg dst, \
	strbne	tmp2, [\B]

.Ldone:
	ldr	tmp2, [sp], #FRAME_SIZE
	cfi_adjust_cfa_offset (-FRAME_SIZE)
	cfi_restore (tmp2)
	bx	lr

	cfi_restore_state
	cfi_remember_state

.Lcpy_body_long:			/* Count in tmp2.  */

	/* Long copy.  We know that there's at least (prefetch_lines * 64)
	   bytes to go.  */
#ifdef USE_VFP
	/* Don't use PLD.  Instead, read some data in advance of the current
	   copy position into a register.  This should act like a PLD
	   operation but we won't have to repeat the transfer.  */

	sfi_breg src, \
	vldr	d3, [\B, #0]
	sfi_breg src, \
	vldr	d4, [\B, #64]
	sfi_breg src, \
	vldr	d5, [\B, #128]
	sfi_breg src, \
	vldr	d6, [\B, #192]
	sfi_breg src, \
	vldr	d7, [\B, #256]

	sfi_breg src, \
	vldr	d0, [\B, #8]
	sfi_breg src, \
	vldr	d1, [\B, #16]
	sfi_breg src, \
	vldr	d2, [\B, #24]
	add	src, src, #32

	subs	tmp2, tmp2, #prefetch_lines * 64 * 2
	blt	2f
1:
	cpy_line_vfp	d3, 0
	cpy_line_vfp	d4, 64
	cpy_line_vfp	d5, 128
	add	dst, dst, #3 * 64
	add	src, src, #3 * 64
	cpy_line_vfp	d6, 0
	cpy_line_vfp	d7, 64
	add	dst, dst, #2 * 64
	add	src, src, #2 * 64
	subs	tmp2, tmp2, #prefetch_lines * 64
	bge	1b

2:
	cpy_tail_vfp	d3, 0
	cpy_tail_vfp	d4, 64
	cpy_tail_vfp	d5, 128
	add	src, src, #3 * 64
	add	dst, dst, #3 * 64
	cpy_tail_vfp	d6, 0
	sfi_breg dst, \
	vstr	d7, [\B, #64]
	sfi_breg src, \
	vldr	d7, [\B, #64]
	sfi_breg dst, \
	vstr	d0, [\B, #64 + 8]
	sfi_breg src, \
	vldr	d0, [\B, #64 + 8]
	sfi_breg dst, \
	vstr	d1, [\B, #64 + 16]
	sfi_breg src, \
	vldr	d1, [\B, #64 + 16]
	sfi_breg dst, \
	vstr	d2, [\B, #64 + 24]
	sfi_breg src, \
	vldr	d2, [\B, #64 + 24]
	sfi_breg dst, \
	vstr	d7, [\B, #64 + 32]
	add	src, src, #96
	sfi_breg dst, \
	vstr	d0, [\B, #64 + 40]
	sfi_breg dst, \
	vstr	d1, [\B, #64 + 48]
	sfi_breg dst, \
	vstr	d2, [\B, #64 + 56]
	add	dst, dst, #128
	add	tmp2, tmp2, #prefetch_lines * 64
	b	.Lcpy_body_medium
#else
	/* Long copy.  Use an SMS style loop to maximize the I/O
	   bandwidth of the core.  We don't have enough spare registers
	   to synthesise prefetching, so use PLD operations.  */
	/* Pre-bias src and dst.  */
	sub	src, src, #8
	sub	dst, dst, #8
	sfi_pld	src, #8
	sfi_pld	src, #72
	subs	tmp2, tmp2, #64
	sfi_pld	src, #136
	sfi_breg src, \
	ldrd	A_l, A_h, [\B, #8]
	strd	B_l, B_h, [sp, #8]
	cfi_rel_offset (B_l, 8)
	cfi_rel_offset (B_h, 12)
	sfi_breg src, \
	ldrd	B_l, B_h, [\B, #16]
	strd	C_l, C_h, [sp, #16]
	cfi_rel_offset (C_l, 16)
	cfi_rel_offset (C_h, 20)
	sfi_breg src, \
	ldrd	C_l, C_h, [\B, #24]
	strd	D_l, D_h, [sp, #24]
	cfi_rel_offset (D_l, 24)
	cfi_rel_offset (D_h, 28)
	sfi_pld	src, #200
	sfi_breg src, \
	ldrd	D_l, D_h, [\B, #32]!
	b	1f
	.p2align	6
2:
	sfi_pld	src, #232
	sfi_breg dst, \
	strd	A_l, A_h, [\B, #40]
	sfi_breg src, \
	ldrd	A_l, A_h, [\B, #40]
	sfi_breg dst, \
	strd	B_l, B_h, [\B, #48]
	sfi_breg src, \
	ldrd	B_l, B_h, [\B, #48]
	sfi_breg dst, \
	strd	C_l, C_h, [\B, #56]
	sfi_breg src, \
	ldrd	C_l, C_h, [\B, #56]
	sfi_breg dst, \
	strd	D_l, D_h, [\B, #64]!
	sfi_breg src, \
	ldrd	D_l, D_h, [\B, #64]!
	subs	tmp2, tmp2, #64
1:
	sfi_breg dst, \
	strd	A_l, A_h, [\B, #8]
	sfi_breg src, \
	ldrd	A_l, A_h, [\B, #8]
	sfi_breg dst, \
	strd	B_l, B_h, [\B, #16]
	sfi_breg src, \
	ldrd	B_l, B_h, [\B, #16]
	sfi_breg dst, \
	strd	C_l, C_h, [\B, #24]
	sfi_breg src, \
	ldrd	C_l, C_h, [\B, #24]
	sfi_breg dst, \
	strd	D_l, D_h, [\B, #32]
	sfi_breg src, \
	ldrd	D_l, D_h, [\B, #32]
	bcs	2b
	/* Save the remaining bytes and restore the callee-saved regs.  */
	sfi_breg dst, \
	strd	A_l, A_h, [\B, #40]
	add	src, src, #40
	sfi_breg dst, \
	strd	B_l, B_h, [\B, #48]
	ldrd	B_l, B_h, [sp, #8]
	cfi_restore (B_l)
	cfi_restore (B_h)
	sfi_breg dst, \
	strd	C_l, C_h, [\B, #56]
	ldrd	C_l, C_h, [sp, #16]
	cfi_restore (C_l)
	cfi_restore (C_h)
	sfi_breg dst, \
	strd	D_l, D_h, [\B, #64]
	ldrd	D_l, D_h, [sp, #24]
	cfi_restore (D_l)
	cfi_restore (D_h)
	add	dst, dst, #72
	tst	tmp2, #0x3f
	bne	.Ltail63aligned
	ldr	tmp2, [sp], #FRAME_SIZE
	cfi_adjust_cfa_offset (-FRAME_SIZE)
	cfi_restore (tmp2)
	bx	lr
#endif

	cfi_restore_state
	cfi_remember_state

.Lcpy_notaligned:
	sfi_pld	src
	sfi_pld	src, #64
	/* There's at least 64 bytes to copy, but there is no mutual
	   alignment.  */
	/* Bring DST to 64-bit alignment.  */
	lsls	tmp2, dst, #29
	sfi_pld	src, #(2 * 64)
	beq	1f
	rsbs	tmp2, tmp2, #0
	sub	count, count, tmp2, lsr #29
	sfi_breg src, \
	ldrmi	tmp1, [\B], #4
	sfi_breg dst, \
	strmi	tmp1, [\B], #4
	lsls	tmp2, tmp2, #2
	sfi_breg src, \
	ldrbne	tmp1, [\B], #1
	sfi_breg src, \
	ldrhcs	tmp2, [\B], #2
	sfi_breg dst, \
	strbne	tmp1, [\B], #1
	sfi_breg dst, \
	strhcs	tmp2, [\B], #2
1:
	sfi_pld	src, #(3 * 64)
	subs	count, count, #64
	ldrmi	tmp2, [sp], #FRAME_SIZE
	bmi	.Ltail63unaligned
	sfi_pld	src, #(4 * 64)

#ifdef USE_NEON
	/* These need an extra layer of macro just to work around a
	   bug in the assembler's parser when an operand starts with
	   a {...}.  */
	.macro neon_load_multi reglist, basereg
	vld1.8	{\reglist}, [\basereg]!
	.endm
	.macro neon_store_multi reglist, basereg
	vst1.8	{\reglist}, [ALIGN (\basereg, 64)]!
	.endm

	/* These are used by the NaCl sfi_breg macro.  */
	.macro _sfi_breg_dmask_neon_load_multi reg
	_sfi_dmask \reg
	.endm
	.macro _sfi_breg_dmask_neon_store_multi reg
	_sfi_dmask \reg
	.endm

	sfi_breg src, neon_load_multi d0-d3, \B
	sfi_breg src, neon_load_multi d4-d7, \B
	subs	count, count, #64
	bmi	2f
1:
	sfi_pld	src, #(4 * 64)
	sfi_breg dst, neon_store_multi d0-d3, \B
	sfi_breg src, neon_load_multi d0-d3, \B
	sfi_breg dst, neon_store_multi d4-d7, \B
	sfi_breg src, neon_load_multi d4-d7, \B
	subs	count, count, #64
	bpl	1b
2:
	sfi_breg dst, neon_store_multi d0-d3, \B
	sfi_breg dst, neon_store_multi d4-d7, \B
	ands	count, count, #0x3f
#else
	/* Use an SMS style loop to maximize the I/O bandwidth.  */
	sub	src, src, #4
	sub	dst, dst, #8
	subs	tmp2, count, #64	/* Use tmp2 for count.  */
	sfi_breg src, \
	ldr	A_l, [\B, #4]
	sfi_breg src, \
	ldr	A_h, [\B, #8]
	strd	B_l, B_h, [sp, #8]
	cfi_rel_offset (B_l, 8)
	cfi_rel_offset (B_h, 12)
	sfi_breg src, \
	ldr	B_l, [\B, #12]
	sfi_breg src, \
	ldr	B_h, [\B, #16]
	strd	C_l, C_h, [sp, #16]
	cfi_rel_offset (C_l, 16)
	cfi_rel_offset (C_h, 20)
	sfi_breg src, \
	ldr	C_l, [\B, #20]
	sfi_breg src, \
	ldr	C_h, [\B, #24]
	strd	D_l, D_h, [sp, #24]
	cfi_rel_offset (D_l, 24)
	cfi_rel_offset (D_h, 28)
	sfi_breg src, \
	ldr	D_l, [\B, #28]
	sfi_breg src, \
	ldr	D_h, [\B, #32]!
	b	1f
	.p2align	6
2:
	sfi_pld	src, #(5 * 64) - (32 - 4)
	sfi_breg dst, \
	strd	A_l, A_h, [\B, #40]
	sfi_breg src, \
	ldr	A_l, [\B, #36]
	sfi_breg src, \
	ldr	A_h, [\B, #40]
	sfi_breg dst, \
	strd	B_l, B_h, [\B, #48]
	sfi_breg src, \
	ldr	B_l, [\B, #44]
	sfi_breg src, \
	ldr	B_h, [\B, #48]
	sfi_breg dst, \
	strd	C_l, C_h, [\B, #56]
	sfi_breg src, \
	ldr	C_l, [\B, #52]
	sfi_breg src, \
	ldr	C_h, [\B, #56]
	sfi_breg dst, \
	strd	D_l, D_h, [\B, #64]!
	sfi_breg src, \
	ldr	D_l, [\B, #60]
	sfi_breg src, \
	ldr	D_h, [\B, #64]!
	subs	tmp2, tmp2, #64
1:
	sfi_breg dst, \
	strd	A_l, A_h, [\B, #8]
	sfi_breg src, \
	ldr	A_l, [\B, #4]
	sfi_breg src, \
	ldr	A_h, [\B, #8]
	sfi_breg dst, \
	strd	B_l, B_h, [\B, #16]
	sfi_breg src, \
	ldr	B_l, [\B, #12]
	sfi_breg src, \
	ldr	B_h, [\B, #16]
	sfi_breg dst, \
	strd	C_l, C_h, [\B, #24]
	sfi_breg src, \
	ldr	C_l, [\B, #20]
	sfi_breg src, \
	ldr	C_h, [\B, #24]
	sfi_breg dst, \
	strd	D_l, D_h, [\B, #32]
	sfi_breg src, \
	ldr	D_l, [\B, #28]
	sfi_breg src, \
	ldr	D_h, [\B, #32]
	bcs	2b

	/* Save the remaining bytes and restore the callee-saved regs.  */
	sfi_breg dst, \
	strd	A_l, A_h, [\B, #40]
	add	src, src, #36
	sfi_breg dst, \
	strd	B_l, B_h, [\B, #48]
	ldrd	B_l, B_h, [sp, #8]
	cfi_restore (B_l)
	cfi_restore (B_h)
	sfi_breg dst, \
	strd	C_l, C_h, [\B, #56]
	ldrd	C_l, C_h, [sp, #16]
	cfi_restore (C_l)
	cfi_restore (C_h)
	sfi_breg dst, \
	strd	D_l, D_h, [\B, #64]
	ldrd	D_l, D_h, [sp, #24]
	cfi_restore (D_l)
	cfi_restore (D_h)
	add	dst, dst, #72
	ands	count, tmp2, #0x3f
#endif
	ldr	tmp2, [sp], #FRAME_SIZE
	cfi_adjust_cfa_offset (-FRAME_SIZE)
	cfi_restore (tmp2)
	bne	.Ltail63unaligned
	bx	lr

END(memcpy)
libc_hidden_builtin_def (memcpy)
