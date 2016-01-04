/* Copyright (C) 2006-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   Contributed by MontaVista Software, Inc. (written by Nicolas Pitre)

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* Thumb requires excessive IT insns here.  */
#define NO_THUMB
#include <sysdep.h>
#include <arm-features.h>

/*
 * Data preload for architectures that support it (ARM V5TE and above)
 */
#if (!defined (__ARM_ARCH_2__) && !defined (__ARM_ARCH_3__) \
     && !defined (__ARM_ARCH_3M__) && !defined (__ARM_ARCH_4__) \
     && !defined (__ARM_ARCH_4T__) && !defined (__ARM_ARCH_5__) \
     && !defined (__ARM_ARCH_5T__))
#define PLD(code...)    code
#else
#define PLD(code...)
#endif

/*
 * This can be used to enable code to cacheline align the source pointer.
 * Experiments on tested architectures (StrongARM and XScale) didn't show
 * this a worthwhile thing to do.  That might be different in the future.
 */
//#define CALGN(code...)        code
#define CALGN(code...)

/*
 * Endian independent macros for shifting bytes within registers.
 */
#ifndef __ARMEB__
#define PULL            lsr
#define PUSH            lsl
#else
#define PULL            lsl
#define PUSH            lsr
#endif

		.text
		.syntax unified

/*
 * Prototype: void *memmove(void *dest, const void *src, size_t n);
 *
 * Note:
 *
 * If the memory regions don't overlap, we simply branch to memcpy which is
 * normally a bit faster. Otherwise the copy is done going downwards.
 */

ENTRY(memmove)

		subs	ip, r0, r1
		cmphi	r2, ip
#if !IS_IN (libc)
		bls	memcpy
#else
		bls	HIDDEN_JUMPTARGET(memcpy)
#endif

		push	{r0, r4, lr}
		cfi_adjust_cfa_offset (12)
		cfi_rel_offset (r4, 4)
		cfi_rel_offset (lr, 8)

		cfi_remember_state

		add	r1, r1, r2
		add	r0, r0, r2
		subs	r2, r2, #4
		blt	8f
		ands	ip, r0, #3
	PLD(	sfi_pld	r1, #-4			)
		bne	9f
		ands	ip, r1, #3
		bne	10f

1:		subs	r2, r2, #(28)
		push	{r5 - r8}
		cfi_adjust_cfa_offset (16)
		cfi_rel_offset (r5, 0)
		cfi_rel_offset (r6, 4)
		cfi_rel_offset (r7, 8)
		cfi_rel_offset (r8, 12)
		blt	5f

	CALGN(	ands	ip, r1, #31		)
	CALGN(	sbcsne	r4, ip, r2		)  @ C is always set here
	CALGN(	bcs	2f			)
	CALGN(	adr	r4, 6f			)
	CALGN(	subs	r2, r2, ip		)  @ C is set here
#ifndef ARM_ALWAYS_BX
	CALGN(	add	pc, r4, ip, lsl	#(ARM_BX_ALIGN_LOG2 - 2))
#else
	CALGN(	add	r4, r4, ip, lsl	#(ARM_BX_ALIGN_LOG2 - 2))
	CALGN(	bx	r4			)
#endif

	PLD(	sfi_pld	r1, #-4			)
2:	PLD(	subs	r2, r2, #96		)
	PLD(	sfi_pld	r1, #-32		)
	PLD(	blt	4f			)
	PLD(	sfi_pld	r1, #-64		)
	PLD(	sfi_pld	r1, #-96		)

3:	PLD(	sfi_pld	r1, #-128		)
4:		sfi_breg r1, \
		ldmdb	\B!, {r3, r4, r5, r6, r7, r8, ip, lr}
		subs	r2, r2, #32
		sfi_breg r0, \
		stmdb	\B!, {r3, r4, r5, r6, r7, r8, ip, lr}
		bge	3b
	PLD(	cmn	r2, #96			)
	PLD(	bge	4b			)

5:		ands	ip, r2, #28
		rsb	ip, ip, #32
#ifndef ARM_ALWAYS_BX
		/* C is always clear here.  */
		addne	pc, pc, ip, lsl #(ARM_BX_ALIGN_LOG2 - 2)
		b	7f
#else
		beq	7f
		push	{r10}
		cfi_adjust_cfa_offset (4)
		cfi_rel_offset (r10, 0)
0:		add	r10, pc, ip, lsl #(ARM_BX_ALIGN_LOG2 - 2)
		/* If alignment is not perfect, then there will be some
		   padding (nop) instructions between this BX and label 6.
		   The computation above assumed that two instructions
		   later is exactly the right spot.  */
		add	r10, #(6f - (0b + PC_OFS))
		bx	r10
#endif
		.p2align ARM_BX_ALIGN_LOG2
6:		nop
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r1, \
		ldr	r3, [\B, #-4]!
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r1, \
		ldr	r4, [\B, #-4]!
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r1, \
		ldr	r5, [\B, #-4]!
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r1, \
		ldr	r6, [\B, #-4]!
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r1, \
		ldr	r7, [\B, #-4]!
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r1, \
		ldr	r8, [\B, #-4]!
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r1, \
		ldr	lr, [\B, #-4]!

#ifndef ARM_ALWAYS_BX
		add	pc, pc, ip, lsl #(ARM_BX_ALIGN_LOG2 - 2)
		nop
#else
0:		add	r10, pc, ip, lsl #(ARM_BX_ALIGN_LOG2 - 2)
		/* If alignment is not perfect, then there will be some
		   padding (nop) instructions between this BX and label 66.
		   The computation above assumed that two instructions
		   later is exactly the right spot.  */
		add	r10, #(66f - (0b + PC_OFS))
		bx	r10
#endif
		.p2align ARM_BX_ALIGN_LOG2
66:		nop
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r0, \
		str	r3, [\B, #-4]!
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r0, \
		str	r4, [\B, #-4]!
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r0, \
		str	r5, [\B, #-4]!
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r0, \
		str	r6, [\B, #-4]!
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r0, \
		str	r7, [\B, #-4]!
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r0, \
		str	r8, [\B, #-4]!
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r0, \
		str	lr, [\B, #-4]!

#ifdef ARM_ALWAYS_BX
		pop	{r10}
		cfi_adjust_cfa_offset (-4)
		cfi_restore (r10)
#endif

	CALGN(	bcs	2b			)

7:		pop	{r5 - r8}
		cfi_adjust_cfa_offset (-16)
		cfi_restore (r5)
		cfi_restore (r6)
		cfi_restore (r7)
		cfi_restore (r8)

8:		movs	r2, r2, lsl #31
		sfi_breg r1, \
		ldrbne	r3, [\B, #-1]!
		sfi_breg r1, \
		ldrbcs	r4, [\B, #-1]!
		sfi_breg r1, \
		ldrbcs	ip, [\B, #-1]
		sfi_breg r0, \
		strbne	r3, [\B, #-1]!
		sfi_breg r0, \
		strbcs	r4, [\B, #-1]!
		sfi_breg r0, \
		strbcs	ip, [\B, #-1]

#if ((defined (__ARM_ARCH_4T__) && defined (__THUMB_INTERWORK__)) \
     || defined (ARM_ALWAYS_BX))
		pop	{r0, r4, lr}
		cfi_adjust_cfa_offset (-12)
		cfi_restore (r4)
		cfi_restore (lr)
		bx      lr
#else
		pop	{r0, r4, pc}
#endif

		cfi_restore_state

9:		cmp	ip, #2
		sfi_breg r1, \
		ldrbgt	r3, [\B, #-1]!
		sfi_breg r1, \
		ldrbge	r4, [\B, #-1]!
		sfi_breg r1, \
		ldrb	lr, [\B, #-1]!
		sfi_breg r0, \
		strbgt	r3, [\B, #-1]!
		sfi_breg r0, \
		strbge	r4, [\B, #-1]!
		subs	r2, r2, ip
		sfi_breg r0, \
		strb	lr, [\B, #-1]!
		blt	8b
		ands	ip, r1, #3
		beq	1b

10:		bic	r1, r1, #3
		cmp	ip, #2
		sfi_breg r1, \
		ldr	r3, [\B, #0]
		beq	17f
		blt	18f


		.macro	backward_copy_shift push pull

		subs	r2, r2, #28
		blt	14f

	CALGN(	ands	ip, r1, #31		)
	CALGN(	rsb	ip, ip, #32		)
	CALGN(	sbcsne	r4, ip, r2		)  @ C is always set here
	CALGN(	subcc	r2, r2, ip		)
	CALGN(	bcc	15f			)

11:		push	{r5 - r8, r10}
		cfi_adjust_cfa_offset (20)
		cfi_rel_offset (r5, 0)
		cfi_rel_offset (r6, 4)
		cfi_rel_offset (r7, 8)
		cfi_rel_offset (r8, 12)
		cfi_rel_offset (r10, 16)

	PLD(	sfi_pld	r1, #-4			)
	PLD(	subs	r2, r2, #96		)
	PLD(	sfi_pld	r1, #-32		)
	PLD(	blt	13f			)
	PLD(	sfi_pld	r1, #-64		)
	PLD(	sfi_pld	r1, #-96		)

12:	PLD(	sfi_pld	r1, #-128		)
13:		sfi_breg r1, \
		ldmdb   \B!, {r7, r8, r10, ip}
		mov     lr, r3, PUSH #\push
		subs    r2, r2, #32
		sfi_breg r1, \
		ldmdb   \B!, {r3, r4, r5, r6}
		orr     lr, lr, ip, PULL #\pull
		mov     ip, ip, PUSH #\push
		orr     ip, ip, r10, PULL #\pull
		mov     r10, r10, PUSH #\push
		orr     r10, r10, r8, PULL #\pull
		mov     r8, r8, PUSH #\push
		orr     r8, r8, r7, PULL #\pull
		mov     r7, r7, PUSH #\push
		orr     r7, r7, r6, PULL #\pull
		mov     r6, r6, PUSH #\push
		orr     r6, r6, r5, PULL #\pull
		mov     r5, r5, PUSH #\push
		orr     r5, r5, r4, PULL #\pull
		mov     r4, r4, PUSH #\push
		orr     r4, r4, r3, PULL #\pull
		sfi_breg r0, \
		stmdb   \B!, {r4 - r8, r10, ip, lr}
		bge	12b
	PLD(	cmn	r2, #96			)
	PLD(	bge	13b			)

		pop	{r5 - r8, r10}
		cfi_adjust_cfa_offset (-20)
		cfi_restore (r5)
		cfi_restore (r6)
		cfi_restore (r7)
		cfi_restore (r8)
		cfi_restore (r10)

14:		ands	ip, r2, #28
		beq	16f

15:		mov     lr, r3, PUSH #\push
		sfi_breg r1, \
		ldr	r3, [\B, #-4]!
		subs	ip, ip, #4
		orr	lr, lr, r3, PULL #\pull
		sfi_breg r0, \
		str	lr, [\B, #-4]!
		bgt	15b
	CALGN(	cmp	r2, #0			)
	CALGN(	bge	11b			)

16:		add	r1, r1, #(\pull / 8)
		b	8b

		.endm


		backward_copy_shift	push=8	pull=24

17:		backward_copy_shift	push=16	pull=16

18:		backward_copy_shift	push=24	pull=8


END(memmove)
libc_hidden_builtin_def (memmove)
