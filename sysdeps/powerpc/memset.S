/* Optimized memset implementation for PowerPC.
   Copyright (C) 1997, 1999, 2000 Free Software Foundation, Inc.
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
#include <bp-sym.h>
#include <bp-asm.h>

/* __ptr_t [r3] memset (__ptr_t s [r3], int c [r4], size_t n [r5]));
   Returns 's'.

   The memset is done in three sizes: byte (8 bits), word (32 bits),
   cache line (256 bits). There is a special case for setting cache lines
   to 0, to take advantage of the dcbz instruction.  */

EALIGN (BP_SYM (memset), 5, 1)

#define rTMP	r0
#define rRTN	r3	/* initial value of 1st argument */
#if __BOUNDED_POINTERS__
# define rMEMP0	r4	/* original value of 1st arg */
# define rCHR	r5	/* char to set in each byte */
# define rLEN	r6	/* length of region to set */
# define rMEMP	r10	/* address at which we are storing */
#else
# define rMEMP0	r3	/* original value of 1st arg */
# define rCHR	r4	/* char to set in each byte */
# define rLEN	r5	/* length of region to set */
# define rMEMP	r6	/* address at which we are storing */
#endif
#define rALIGN	r7	/* number of bytes we are setting now (when aligning) */
#define rMEMP2	r8

#define rPOS32	r7	/* constant +32 for clearing with dcbz */
#define rNEG64	r8	/* constant -64 for clearing with dcbz */
#define rNEG32	r9	/* constant -32 for clearing with dcbz */

#if __BOUNDED_POINTERS__
	cmplwi	cr1, rRTN, 0
	CHECK_BOUNDS_BOTH_WIDE (rMEMP0, rTMP, rTMP2, rLEN)
	beq	cr1, L(b0)
	STORE_RETURN_VALUE (rMEMP0)
	STORE_RETURN_BOUNDS (rTMP, rTMP2)
L(b0):
#endif

/* take care of case for size <= 4  */
	cmplwi	cr1, rLEN, 4
	andi.	rALIGN, rMEMP0, 3
	mr	rMEMP, rMEMP0
	ble-	cr1, L(small)
/* align to word boundary  */
	cmplwi	cr5, rLEN, 31
	rlwimi	rCHR, rCHR, 8, 16, 23
	beq+	L(aligned)	/* 8th instruction from .align */
	mtcrf	0x01, rMEMP0
	subfic	rALIGN, rALIGN, 4
	add	rMEMP, rMEMP, rALIGN
	sub	rLEN, rLEN, rALIGN
	bf+	31, L(g0)
	stb	rCHR, 0(rMEMP0)
	bt	30, L(aligned)
L(g0):	sth	rCHR, -2(rMEMP)	/* 16th instruction from .align */
/* take care of case for size < 31 */
L(aligned):
	mtcrf	0x01, rLEN
	rlwimi	rCHR, rCHR, 16, 0, 15
	ble	cr5, L(medium)
/* align to cache line boundary...  */
	andi.	rALIGN, rMEMP, 0x1C
	subfic	rALIGN, rALIGN, 0x20
	beq	L(caligned)
	mtcrf	0x01, rALIGN
	add	rMEMP, rMEMP, rALIGN
	sub	rLEN, rLEN, rALIGN
	cmplwi	cr1, rALIGN, 0x10
	mr	rMEMP2, rMEMP
	bf	28, L(a1)
	stw	rCHR, -4(rMEMP2)
	stwu	rCHR, -8(rMEMP2)
L(a1):	blt	cr1, L(a2)
	stw	rCHR, -4(rMEMP2) /* 32nd instruction from .align */
	stw	rCHR, -8(rMEMP2)
	stw	rCHR, -12(rMEMP2)
	stwu	rCHR, -16(rMEMP2)
L(a2):	bf	29, L(caligned)
	stw	rCHR, -4(rMEMP2)
/* now aligned to a cache line.  */
L(caligned):
	cmplwi	cr1, rCHR, 0
	clrrwi.	rALIGN, rLEN, 5
	mtcrf	0x01, rLEN	/* 40th instruction from .align */
	beq	cr1, L(zloopstart) /* special case for clearing memory using dcbz */
	srwi	rTMP, rALIGN, 5
	mtctr	rTMP
	beq	L(medium)	/* we may not actually get to do a full line */
	clrlwi.	rLEN, rLEN, 27
	add	rMEMP, rMEMP, rALIGN
	li	rNEG64, -0x40
	bdz	L(cloopdone)	/* 48th instruction from .align */

L(c3):	dcbz	rNEG64, rMEMP
	stw	rCHR, -4(rMEMP)
	stw	rCHR, -8(rMEMP)
	stw	rCHR, -12(rMEMP)
	stw	rCHR, -16(rMEMP)
	nop			/* let 601 fetch last 4 instructions of loop */
	stw	rCHR, -20(rMEMP)
	stw	rCHR, -24(rMEMP) /* 56th instruction from .align */
	nop			/* let 601 fetch first 8 instructions of loop */
	stw	rCHR, -28(rMEMP)
	stwu	rCHR, -32(rMEMP)
	bdnz	L(c3)
L(cloopdone):
	stw	rCHR, -4(rMEMP)
	stw	rCHR, -8(rMEMP)
	stw	rCHR, -12(rMEMP)
	stw	rCHR, -16(rMEMP) /* 64th instruction from .align */
	stw	rCHR, -20(rMEMP)
	cmplwi	cr1, rLEN, 16
	stw	rCHR, -24(rMEMP)
	stw	rCHR, -28(rMEMP)
	stwu	rCHR, -32(rMEMP)
	beqlr
	add	rMEMP, rMEMP, rALIGN
	b	L(medium_tail2)	/* 72nd instruction from .align */

	.align 5
	nop
/* Clear lines of memory in 128-byte chunks.  */
L(zloopstart):
	clrlwi	rLEN, rLEN, 27
	mtcrf	0x02, rALIGN
	srwi.	rTMP, rALIGN, 7
	mtctr	rTMP
	li	rPOS32, 0x20
	li	rNEG64, -0x40
	cmplwi	cr1, rLEN, 16	/* 8 */
	bf	26, L(z0)
	dcbz	0, rMEMP
	addi	rMEMP, rMEMP, 0x20
L(z0):	li	rNEG32, -0x20
	bf	25, L(z1)
	dcbz	0, rMEMP
	dcbz	rPOS32, rMEMP
	addi	rMEMP, rMEMP, 0x40 /* 16 */
L(z1):	cmplwi	cr5, rLEN, 0
	beq	L(medium)
L(zloop):
	dcbz	0, rMEMP
	dcbz	rPOS32, rMEMP
	addi	rMEMP, rMEMP, 0x80
	dcbz	rNEG64, rMEMP
	dcbz	rNEG32, rMEMP
	bdnz	L(zloop)
	beqlr	cr5
	b	L(medium_tail2)

	.align 5
L(small):
/* Memset of 4 bytes or less.  */
	cmplwi	cr5, rLEN, 1
	cmplwi	cr1, rLEN, 3
	bltlr	cr5
	stb	rCHR, 0(rMEMP)
	beqlr	cr5
	nop
	stb	rCHR, 1(rMEMP)
	bltlr	cr1
	stb	rCHR, 2(rMEMP)
	beqlr	cr1
	nop
	stb	rCHR, 3(rMEMP)
	blr

/* Memset of 0-31 bytes.  */
	.align 5
L(medium):
	cmplwi	cr1, rLEN, 16
L(medium_tail2):
	add	rMEMP, rMEMP, rLEN
L(medium_tail):
	bt-	31, L(medium_31t)
	bt-	30, L(medium_30t)
L(medium_30f):
	bt-	29, L(medium_29t)
L(medium_29f):
	bge-	cr1, L(medium_27t)
	bflr-	28
	stw	rCHR, -4(rMEMP)	/* 8th instruction from .align */
	stw	rCHR, -8(rMEMP)
	blr

L(medium_31t):
	stbu	rCHR, -1(rMEMP)
	bf-	30, L(medium_30f)
L(medium_30t):
	sthu	rCHR, -2(rMEMP)
	bf-	29, L(medium_29f)
L(medium_29t):
	stwu	rCHR, -4(rMEMP)
	blt-	cr1, L(medium_27f) /* 16th instruction from .align */
L(medium_27t):
	stw	rCHR, -4(rMEMP)
	stw	rCHR, -8(rMEMP)
	stw	rCHR, -12(rMEMP)
	stwu	rCHR, -16(rMEMP)
L(medium_27f):
	bflr-	28
L(medium_28t):
	stw	rCHR, -4(rMEMP)
	stw	rCHR, -8(rMEMP)
	blr
END (BP_SYM (memset))
