/* Copyright (C) 2006, 2008, 2009 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>

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
#define pull            lsr
#define push            lsl
#else
#define pull            lsl
#define push            lsr
#endif

		.text

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
#ifdef NOT_IN_libc
		bls	memcpy
#else
		bls	HIDDEN_JUMPTARGET(memcpy)
#endif

		stmfd	sp!, {r0, r4, lr}
		cfi_adjust_cfa_offset (12)
		cfi_rel_offset (r4, 4)
		cfi_rel_offset (lr, 8)

		cfi_remember_state

		add	r1, r1, r2
		add	r0, r0, r2
		subs	r2, r2, #4
		blt	8f
		ands	ip, r0, #3
	PLD(	pld	[r1, #-4]		)
		bne	9f
		ands	ip, r1, #3
		bne	10f

1:		subs	r2, r2, #(28)
		stmfd	sp!, {r5 - r8}
		cfi_adjust_cfa_offset (16)
		cfi_rel_offset (r5, 0)
		cfi_rel_offset (r6, 4)
		cfi_rel_offset (r7, 8)
		cfi_rel_offset (r8, 12)
		blt	5f

	CALGN(	ands	ip, r1, #31		)
	CALGN(	sbcnes	r4, ip, r2		)  @ C is always set here
	CALGN(	bcs	2f			)
	CALGN(	adr	r4, 6f			)
	CALGN(	subs	r2, r2, ip		)  @ C is set here
	CALGN(	add	pc, r4, ip		)

	PLD(	pld	[r1, #-4]		)
2:	PLD(	subs	r2, r2, #96		)
	PLD(	pld	[r1, #-32]		)
	PLD(	blt	4f			)
	PLD(	pld	[r1, #-64]		)
	PLD(	pld	[r1, #-96]		)

3:	PLD(	pld	[r1, #-128]		)
4:		ldmdb	r1!, {r3, r4, r5, r6, r7, r8, ip, lr}
		subs	r2, r2, #32
		stmdb	r0!, {r3, r4, r5, r6, r7, r8, ip, lr}
		bge	3b
	PLD(	cmn	r2, #96			)
	PLD(	bge	4b			)

5:		ands	ip, r2, #28
		rsb	ip, ip, #32
		addne	pc, pc, ip		@ C is always clear here
		b	7f
6:		nop
		ldr	r3, [r1, #-4]!
		ldr	r4, [r1, #-4]!
		ldr	r5, [r1, #-4]!
		ldr	r6, [r1, #-4]!
		ldr	r7, [r1, #-4]!
		ldr	r8, [r1, #-4]!
		ldr	lr, [r1, #-4]!

		add	pc, pc, ip
		nop
		nop
		str	r3, [r0, #-4]!
		str	r4, [r0, #-4]!
		str	r5, [r0, #-4]!
		str	r6, [r0, #-4]!
		str	r7, [r0, #-4]!
		str	r8, [r0, #-4]!
		str	lr, [r0, #-4]!

	CALGN(	bcs	2b			)

7:		ldmfd	sp!, {r5 - r8}
		cfi_adjust_cfa_offset (-16)
		cfi_restore (r5)
		cfi_restore (r6)
		cfi_restore (r7)
		cfi_restore (r8)

8:		movs	r2, r2, lsl #31
		ldrneb	r3, [r1, #-1]!
		ldrcsb	r4, [r1, #-1]!
		ldrcsb	ip, [r1, #-1]
		strneb	r3, [r0, #-1]!
		strcsb	r4, [r0, #-1]!
		strcsb	ip, [r0, #-1]

#if defined (__ARM_ARCH_4T__) && defined (__THUMB_INTERWORK__)
		ldmfd	sp!, {r0, r4, lr}
		cfi_adjust_cfa_offset (-12)
		cfi_restore (r4)
		cfi_restore (lr)
		bx      lr
#else
		ldmfd	sp!, {r0, r4, pc}
#endif

		cfi_restore_state

9:		cmp	ip, #2
		ldrgtb	r3, [r1, #-1]!
		ldrgeb	r4, [r1, #-1]!
		ldrb	lr, [r1, #-1]!
		strgtb	r3, [r0, #-1]!
		strgeb	r4, [r0, #-1]!
		subs	r2, r2, ip
		strb	lr, [r0, #-1]!
		blt	8b
		ands	ip, r1, #3
		beq	1b

10:		bic	r1, r1, #3
		cmp	ip, #2
		ldr	r3, [r1, #0]
		beq	17f
		blt	18f


		.macro	backward_copy_shift push pull

		subs	r2, r2, #28
		blt	14f

	CALGN(	ands	ip, r1, #31		)
	CALGN(	rsb	ip, ip, #32		)
	CALGN(	sbcnes	r4, ip, r2		)  @ C is always set here
	CALGN(	subcc	r2, r2, ip		)
	CALGN(	bcc	15f			)

11:		stmfd	sp!, {r5 - r9}
		cfi_adjust_cfa_offset (20)
		cfi_rel_offset (r5, 0)
		cfi_rel_offset (r6, 4)
		cfi_rel_offset (r7, 8)
		cfi_rel_offset (r8, 12)
		cfi_rel_offset (r9, 16)

	PLD(	pld	[r1, #-4]		)
	PLD(	subs	r2, r2, #96		)
	PLD(	pld	[r1, #-32]		)
	PLD(	blt	13f			)
	PLD(	pld	[r1, #-64]		)
	PLD(	pld	[r1, #-96]		)

12:	PLD(	pld	[r1, #-128]		)
13:		ldmdb   r1!, {r7, r8, r9, ip}
		mov     lr, r3, push #\push
		subs    r2, r2, #32
		ldmdb   r1!, {r3, r4, r5, r6}
		orr     lr, lr, ip, pull #\pull
		mov     ip, ip, push #\push
		orr     ip, ip, r9, pull #\pull
		mov     r9, r9, push #\push
		orr     r9, r9, r8, pull #\pull
		mov     r8, r8, push #\push
		orr     r8, r8, r7, pull #\pull
		mov     r7, r7, push #\push
		orr     r7, r7, r6, pull #\pull
		mov     r6, r6, push #\push
		orr     r6, r6, r5, pull #\pull
		mov     r5, r5, push #\push
		orr     r5, r5, r4, pull #\pull
		mov     r4, r4, push #\push
		orr     r4, r4, r3, pull #\pull
		stmdb   r0!, {r4 - r9, ip, lr}
		bge	12b
	PLD(	cmn	r2, #96			)
	PLD(	bge	13b			)

		ldmfd	sp!, {r5 - r9}
		cfi_adjust_cfa_offset (-20)
		cfi_restore (r5)
		cfi_restore (r6)
		cfi_restore (r7)
		cfi_restore (r8)
		cfi_restore (r9)

14:		ands	ip, r2, #28
		beq	16f

15:		mov     lr, r3, push #\push
		ldr	r3, [r1, #-4]!
		subs	ip, ip, #4
		orr	lr, lr, r3, pull #\pull
		str	lr, [r0, #-4]!
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
