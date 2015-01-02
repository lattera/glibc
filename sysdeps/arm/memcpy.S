/* Copyright (C) 2006-2015 Free Software Foundation, Inc.
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

/* Prototype: void *memcpy(void *dest, const void *src, size_t n); */

ENTRY(memcpy)

		push	{r0, r4, lr}
		cfi_adjust_cfa_offset (12)
		cfi_rel_offset (r4, 4)
		cfi_rel_offset (lr, 8)

		cfi_remember_state

		subs	r2, r2, #4
		blt	8f
		ands	ip, r0, #3
	PLD(	sfi_pld	r1, #0			)
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
	CALGN(	rsb	r3, ip, #32		)
	CALGN(	sbcsne	r4, r3, r2		)  @ C is always set here
	CALGN(	bcs	2f			)
	CALGN(	adr	r4, 6f			)
	CALGN(	subs	r2, r2, r3		)  @ C gets set
#ifndef ARM_ALWAYS_BX
	CALGN(	add	pc, r4, ip, lsl	#(ARM_BX_ALIGN_LOG2 - 2))
#else
	CALGN(	add	r4, r4, ip, lsl	#(ARM_BX_ALIGN_LOG2 - 2))
	CALGN(	bx	r4			)
#endif

	PLD(	sfi_pld	r1, #0			)
2:	PLD(	subs	r2, r2, #96		)
	PLD(	sfi_pld	r1, #28			)
	PLD(	blt	4f			)
	PLD(	sfi_pld	r1, #60			)
	PLD(	sfi_pld	r1, #92			)

3:	PLD(	sfi_pld	r1, #124		)
4:		sfi_breg r1, \
		ldmia	\B!, {r3, r4, r5, r6, r7, r8, ip, lr}
		subs	r2, r2, #32
		sfi_breg r0, \
		stmia	\B!, {r3, r4, r5, r6, r7, r8, ip, lr}
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
		add	r10, pc, ip, lsl #(ARM_BX_ALIGN_LOG2 - 2)
		bx	r10
#endif
		.p2align ARM_BX_ALIGN_LOG2
6:		nop
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r1, \
		ldr	r3, [\B], #4
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r1, \
		ldr	r4, [\B], #4
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r1, \
		ldr	r5, [\B], #4
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r1, \
		ldr	r6, [\B], #4
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r1, \
		ldr	r7, [\B], #4
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r1, \
		ldr	r8, [\B], #4
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r1, \
		ldr	lr, [\B], #4

#ifndef ARM_ALWAYS_BX
		add	pc, pc, ip, lsl #(ARM_BX_ALIGN_LOG2 - 2)
		nop
#else
		add	r10, pc, ip, lsl #(ARM_BX_ALIGN_LOG2 - 2)
		bx	r10
#endif
		.p2align ARM_BX_ALIGN_LOG2
		nop
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r0, \
		str	r3, [\B], #4
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r0, \
		str	r4, [\B], #4
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r0, \
		str	r5, [\B], #4
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r0, \
		str	r6, [\B], #4
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r0, \
		str	r7, [\B], #4
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r0, \
		str	r8, [\B], #4
		.p2align ARM_BX_ALIGN_LOG2
		sfi_breg r0, \
		str	lr, [\B], #4

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
		ldrbne	r3, [\B], #1
		sfi_breg r1, \
		ldrbcs	r4, [\B], #1
		sfi_breg r1, \
		ldrbcs	ip, [\B]
		sfi_breg r0, \
		strbne	r3, [\B], #1
		sfi_breg r0, \
		strbcs	r4, [\B], #1
		sfi_breg r0, \
		strbcs	ip, [\B]

#if ((defined (__ARM_ARCH_4T__) && defined(__THUMB_INTERWORK__)) \
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

9:		rsb	ip, ip, #4
		cmp	ip, #2
		sfi_breg r1, \
		ldrbgt	r3, [\B], #1
		sfi_breg r1, \
		ldrbge	r4, [\B], #1
		sfi_breg r1, \
		ldrb	lr, [\B], #1
		sfi_breg r0, \
		strbgt	r3, [\B], #1
		sfi_breg r0, \
		strbge	r4, [\B], #1
		subs	r2, r2, ip
		sfi_breg r0, \
		strb	lr, [\B], #1
		blt	8b
		ands	ip, r1, #3
		beq	1b

10:		bic	r1, r1, #3
		cmp	ip, #2
		sfi_breg r1, \
		ldr	lr, [\B], #4
		beq	17f
		bgt	18f


		.macro	forward_copy_shift pull push

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

	PLD(	sfi_pld	r1, #0			)
	PLD(	subs	r2, r2, #96		)
	PLD(	sfi_pld	r1, #28			)
	PLD(	blt	13f			)
	PLD(	sfi_pld	r1, #60			)
	PLD(	sfi_pld	r1, #92			)

12:	PLD(	sfi_pld	r1, #124		)
13:		sfi_breg r1, \
		ldmia	\B!, {r4, r5, r6, r7}
		mov	r3, lr, PULL #\pull
		subs	r2, r2, #32
		sfi_breg r1, \
		ldmia	\B!, {r8, r10, ip, lr}
		orr	r3, r3, r4, PUSH #\push
		mov	r4, r4, PULL #\pull
		orr	r4, r4, r5, PUSH #\push
		mov	r5, r5, PULL #\pull
		orr	r5, r5, r6, PUSH #\push
		mov	r6, r6, PULL #\pull
		orr	r6, r6, r7, PUSH #\push
		mov	r7, r7, PULL #\pull
		orr	r7, r7, r8, PUSH #\push
		mov	r8, r8, PULL #\pull
		orr	r8, r8, r10, PUSH #\push
		mov	r10, r10, PULL #\pull
		orr	r10, r10, ip, PUSH #\push
		mov	ip, ip, PULL #\pull
		orr	ip, ip, lr, PUSH #\push
		sfi_breg r0, \
		stmia	\B!, {r3, r4, r5, r6, r7, r8, r10, ip}
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

15:		mov	r3, lr, PULL #\pull
		sfi_breg r1, \
		ldr	lr, [\B], #4
		subs	ip, ip, #4
		orr	r3, r3, lr, PUSH #\push
		sfi_breg r0, \
		str	r3, [\B], #4
		bgt	15b
	CALGN(	cmp	r2, #0			)
	CALGN(	bge	11b			)

16:		sub	r1, r1, #(\push / 8)
		b	8b

		.endm


		forward_copy_shift	pull=8	push=24

17:		forward_copy_shift	pull=16	push=16

18:		forward_copy_shift	pull=24	push=8

END(memcpy)
libc_hidden_builtin_def (memcpy)
