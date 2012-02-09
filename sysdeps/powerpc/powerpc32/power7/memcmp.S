/* Optimized memcmp implementation for POWER7/PowerPC32.
   Copyright (C) 2010 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <bp-sym.h>
#include <bp-asm.h>

/* int [r3] memcmp (const char *s1 [r3],
		    const char *s2 [r4],
		    size_t size [r5])  */

	.machine power7
EALIGN (BP_SYM(memcmp),4,0)
	CALL_MCOUNT

#define rTMP	r0
#define rRTN	r3
#define rSTR1	r3	/* first string arg */
#define rSTR2	r4	/* second string arg */
#define rN	r5	/* max string length */
#define rWORD1	r6	/* current word in s1 */
#define rWORD2	r7	/* current word in s2 */
#define rWORD3	r8	/* next word in s1 */
#define rWORD4	r9	/* next word in s2 */
#define rWORD5	r10	/* next word in s1 */
#define rWORD6	r11	/* next word in s2 */
#define rBITDIF	r12	/* bits that differ in s1 & s2 words */
#define rWORD7	r30	/* next word in s1 */
#define rWORD8	r31	/* next word in s2 */

	xor	rTMP,rSTR2,rSTR1
	cmplwi	cr6,rN,0
	cmplwi	cr1,rN,12
	clrlwi.	rTMP,rTMP,30
	clrlwi	rBITDIF,rSTR1,30
	cmplwi	cr5,rBITDIF,0
	beq-	cr6,L(zeroLength)
	dcbt	0,rSTR1
	dcbt	0,rSTR2

	/* If less than 8 bytes or not aligned, use the unaligned
	   byte loop.  */

	blt	cr1,L(bytealigned)
	stwu	1,-64(1)
	cfi_adjust_cfa_offset(64)
	stw	r31,48(1)
	cfi_offset(31,(48-64))
	stw	r30,44(1)
	cfi_offset(30,(44-64))
	bne	L(unaligned)
/* At this point we know both strings have the same alignment and the
   compare length is at least 8 bytes.  rBITDIF contains the low order
   2 bits of rSTR1 and cr5 contains the result of the logical compare
   of rBITDIF to 0.  If rBITDIF == 0 then we are already word
   aligned and can perform the word aligned loop.

   Otherwise we know the two strings have the same alignment (but not
   yet word aligned).  So we force the string addresses to the next lower
   word boundary and special case this first word using shift left to
   eliminate bits preceeding the first byte.  Since we want to join the
   normal (word aligned) compare loop, starting at the second word,
   we need to adjust the length (rN) and special case the loop
   versioning for the first word. This insures that the loop count is
   correct and the first word (shifted) is in the expected register pair. */
	.align	4
L(samealignment):
	clrrwi	rSTR1,rSTR1,2
	clrrwi	rSTR2,rSTR2,2
	beq	cr5,L(Waligned)
	add	rN,rN,rBITDIF
	slwi	r11,rBITDIF,3
	srwi	rTMP,rN,4	/* Divide by 16 */
	andi.	rBITDIF,rN,12	/* Get the word remainder */
	lwz	rWORD1,0(rSTR1)
	lwz	rWORD2,0(rSTR2)
	cmplwi	cr1,rBITDIF,8
	cmplwi	cr7,rN,16
	clrlwi	rN,rN,30
	beq	L(dPs4)
	mtctr	rTMP
	bgt	cr1,L(dPs3)
	beq	cr1,L(dPs2)

/* Remainder is 4 */
	.align	3
L(dsP1):
	slw	rWORD5,rWORD1,r11
	slw	rWORD6,rWORD2,r11
	cmplw	cr5,rWORD5,rWORD6
	blt	cr7,L(dP1x)
/* Do something useful in this cycle since we have to branch anyway.  */
	lwz	rWORD1,4(rSTR1)
	lwz	rWORD2,4(rSTR2)
	cmplw	cr0,rWORD1,rWORD2
	b	L(dP1e)
/* Remainder is 8 */
	.align	4
L(dPs2):
	slw	rWORD5,rWORD1,r11
	slw	rWORD6,rWORD2,r11
	cmplw	cr6,rWORD5,rWORD6
	blt	cr7,L(dP2x)
/* Do something useful in this cycle since we have to branch anyway.  */
	lwz	rWORD7,4(rSTR1)
	lwz	rWORD8,4(rSTR2)
	cmplw	cr5,rWORD7,rWORD8
	b	L(dP2e)
/* Remainder is 12 */
	.align	4
L(dPs3):
	slw	rWORD3,rWORD1,r11
	slw	rWORD4,rWORD2,r11
	cmplw	cr1,rWORD3,rWORD4
	b	L(dP3e)
/* Count is a multiple of 16, remainder is 0 */
	.align	4
L(dPs4):
	mtctr	rTMP
	slw	rWORD1,rWORD1,r11
	slw	rWORD2,rWORD2,r11
	cmplw	cr0,rWORD1,rWORD2
	b	L(dP4e)

/* At this point we know both strings are word aligned and the
   compare length is at least 8 bytes.  */
	.align	4
L(Waligned):
	andi.	rBITDIF,rN,12	/* Get the word remainder */
	srwi	rTMP,rN,4	/* Divide by 16 */
	cmplwi	cr1,rBITDIF,8
	cmplwi	cr7,rN,16
	clrlwi	rN,rN,30
	beq	L(dP4)
	bgt	cr1,L(dP3)
	beq	cr1,L(dP2)

/* Remainder is 4 */
	.align	4
L(dP1):
	mtctr	rTMP
/* Normally we'd use rWORD7/rWORD8 here, but since we might exit early
   (8-15 byte compare), we want to use only volatile registers.  This
   means we can avoid restoring non-volatile registers since we did not
   change any on the early exit path.  The key here is the non-early
   exit path only cares about the condition code (cr5), not about which
   register pair was used.  */
	lwz	rWORD5,0(rSTR1)
	lwz	rWORD6,0(rSTR2)
	cmplw	cr5,rWORD5,rWORD6
	blt	cr7,L(dP1x)
	lwz	rWORD1,4(rSTR1)
	lwz	rWORD2,4(rSTR2)
	cmplw	cr0,rWORD1,rWORD2
L(dP1e):
	lwz	rWORD3,8(rSTR1)
	lwz	rWORD4,8(rSTR2)
	cmplw	cr1,rWORD3,rWORD4
	lwz	rWORD5,12(rSTR1)
	lwz	rWORD6,12(rSTR2)
	cmplw	cr6,rWORD5,rWORD6
	bne	cr5,L(dLcr5)
	bne	cr0,L(dLcr0)

	lwzu	rWORD7,16(rSTR1)
	lwzu	rWORD8,16(rSTR2)
	bne	cr1,L(dLcr1)
	cmplw	cr5,rWORD7,rWORD8
	bdnz	L(dLoop)
	bne	cr6,L(dLcr6)
	lwz	r30,44(1)
	lwz	r31,48(1)
	.align	3
L(dP1x):
	slwi.	r12,rN,3
	bne	cr5,L(dLcr5)
	subfic	rN,r12,32	/* Shift count is 32 - (rN * 8).  */
	lwz	1,0(1)
	bne	L(d00)
	li	rRTN,0
	blr

/* Remainder is 8 */
	.align	4
L(dP2):
	mtctr	rTMP
	lwz	rWORD5,0(rSTR1)
	lwz	rWORD6,0(rSTR2)
	cmplw	cr6,rWORD5,rWORD6
	blt	cr7,L(dP2x)
	lwz	rWORD7,4(rSTR1)
	lwz	rWORD8,4(rSTR2)
	cmplw	cr5,rWORD7,rWORD8
L(dP2e):
	lwz	rWORD1,8(rSTR1)
	lwz	rWORD2,8(rSTR2)
	cmplw	cr0,rWORD1,rWORD2
	lwz	rWORD3,12(rSTR1)
	lwz	rWORD4,12(rSTR2)
	cmplw	cr1,rWORD3,rWORD4
	addi	rSTR1,rSTR1,4
	addi	rSTR2,rSTR2,4
	bne	cr6,L(dLcr6)
	bne	cr5,L(dLcr5)
	b	L(dLoop2)
/* Again we are on a early exit path (16-23 byte compare), we want to
   only use volatile registers and avoid restoring non-volatile
   registers.  */
	.align	4
L(dP2x):
	lwz	rWORD3,4(rSTR1)
	lwz	rWORD4,4(rSTR2)
	cmplw	cr5,rWORD3,rWORD4
	slwi.	r12,rN,3
	bne	cr6,L(dLcr6)
	addi	rSTR1,rSTR1,4
	addi	rSTR2,rSTR2,4
	bne	cr5,L(dLcr5)
	subfic	rN,r12,32	/* Shift count is 32 - (rN * 8).  */
	lwz	1,0(1)
	bne	L(d00)
	li	rRTN,0
	blr

/* Remainder is 12 */
	.align	4
L(dP3):
	mtctr	rTMP
	lwz	rWORD3,0(rSTR1)
	lwz	rWORD4,0(rSTR2)
	cmplw	cr1,rWORD3,rWORD4
L(dP3e):
	lwz	rWORD5,4(rSTR1)
	lwz	rWORD6,4(rSTR2)
	cmplw	cr6,rWORD5,rWORD6
	blt	cr7,L(dP3x)
	lwz	rWORD7,8(rSTR1)
	lwz	rWORD8,8(rSTR2)
	cmplw	cr5,rWORD7,rWORD8
	lwz	rWORD1,12(rSTR1)
	lwz	rWORD2,12(rSTR2)
	cmplw	cr0,rWORD1,rWORD2
	addi	rSTR1,rSTR1,8
	addi	rSTR2,rSTR2,8
	bne	cr1,L(dLcr1)
	bne	cr6,L(dLcr6)
	b	L(dLoop1)
/* Again we are on a early exit path (24-31 byte compare), we want to
   only use volatile registers and avoid restoring non-volatile
   registers.  */
	.align	4
L(dP3x):
	lwz	rWORD1,8(rSTR1)
	lwz	rWORD2,8(rSTR2)
	cmplw	cr5,rWORD1,rWORD2
	slwi.	r12,rN,3
	bne	cr1,L(dLcr1)
	addi	rSTR1,rSTR1,8
	addi	rSTR2,rSTR2,8
	bne	cr6,L(dLcr6)
	subfic	rN,r12,32	/* Shift count is 32 - (rN * 8).  */
	bne	cr5,L(dLcr5)
	lwz	1,0(1)
	bne	L(d00)
	li	rRTN,0
	blr

/* Count is a multiple of 16, remainder is 0 */
	.align	4
L(dP4):
	mtctr	rTMP
	lwz	rWORD1,0(rSTR1)
	lwz	rWORD2,0(rSTR2)
	cmplw	cr0,rWORD1,rWORD2
L(dP4e):
	lwz	rWORD3,4(rSTR1)
	lwz	rWORD4,4(rSTR2)
	cmplw	cr1,rWORD3,rWORD4
	lwz	rWORD5,8(rSTR1)
	lwz	rWORD6,8(rSTR2)
	cmplw	cr6,rWORD5,rWORD6
	lwzu	rWORD7,12(rSTR1)
	lwzu	rWORD8,12(rSTR2)
	cmplw	cr5,rWORD7,rWORD8
	bne	cr0,L(dLcr0)
	bne	cr1,L(dLcr1)
	bdz-	L(d24)		/* Adjust CTR as we start with +4 */
/* This is the primary loop */
	.align	4
L(dLoop):
	lwz	rWORD1,4(rSTR1)
	lwz	rWORD2,4(rSTR2)
	cmplw	cr1,rWORD3,rWORD4
	bne	cr6,L(dLcr6)
L(dLoop1):
	lwz	rWORD3,8(rSTR1)
	lwz	rWORD4,8(rSTR2)
	cmplw	cr6,rWORD5,rWORD6
	bne	cr5,L(dLcr5)
L(dLoop2):
	lwz	rWORD5,12(rSTR1)
	lwz	rWORD6,12(rSTR2)
	cmplw	cr5,rWORD7,rWORD8
	bne	cr0,L(dLcr0)
L(dLoop3):
	lwzu	rWORD7,16(rSTR1)
	lwzu	rWORD8,16(rSTR2)
	bne	cr1,L(dLcr1)
	cmplw	cr0,rWORD1,rWORD2
	bdnz	L(dLoop)

L(dL4):
	cmplw	cr1,rWORD3,rWORD4
	bne	cr6,L(dLcr6)
	cmplw	cr6,rWORD5,rWORD6
	bne	cr5,L(dLcr5)
	cmplw	cr5,rWORD7,rWORD8
L(d44):
	bne	cr0,L(dLcr0)
L(d34):
	bne	cr1,L(dLcr1)
L(d24):
	bne	cr6,L(dLcr6)
L(d14):
	slwi.	r12,rN,3
	bne	cr5,L(dLcr5)
L(d04):
	lwz	r30,44(1)
	lwz	r31,48(1)
	lwz	1,0(1)
	subfic	rN,r12,32	/* Shift count is 32 - (rN * 8).  */
	beq	L(zeroLength)
/* At this point we have a remainder of 1 to 3 bytes to compare.  Since
   we are aligned it is safe to load the whole word, and use
   shift right to eliminate bits beyond the compare length. */
L(d00):
	lwz	rWORD1,4(rSTR1)
	lwz	rWORD2,4(rSTR2)
	srw	rWORD1,rWORD1,rN
	srw	rWORD2,rWORD2,rN
	cmplw	rWORD1,rWORD2
	li	rRTN,0
	beqlr
	li	rRTN,1
	bgtlr
	li	rRTN,-1
	blr

	.align	4
L(dLcr0):
	lwz	r30,44(1)
	lwz	r31,48(1)
	li	rRTN,1
	lwz	1,0(1)
	bgtlr	cr0
	li	rRTN,-1
	blr
	.align	4
L(dLcr1):
	lwz	r30,44(1)
	lwz	r31,48(1)
	li	rRTN,1
	lwz	1,0(1)
	bgtlr	cr1
	li	rRTN,-1
	blr
	.align	4
L(dLcr6):
	lwz	r30,44(1)
	lwz	r31,48(1)
	li	rRTN,1
	lwz	1,0(1)
	bgtlr	cr6
	li	rRTN,-1
	blr
	.align	4
L(dLcr5):
	lwz	r30,44(1)
	lwz	r31,48(1)
L(dLcr5x):
	li	rRTN,1
	lwz	1,0(1)
	bgtlr	cr5
	li	rRTN,-1
	blr

	.align	4
L(bytealigned):
	cfi_adjust_cfa_offset(-64)
	mtctr	rN

/* We need to prime this loop.  This loop is swing modulo scheduled
   to avoid pipe delays.  The dependent instruction latencies (load to
   compare to conditional branch) is 2 to 3 cycles.  In this loop each
   dispatch group ends in a branch and takes 1 cycle.  Effectively
   the first iteration of the loop only serves to load operands and
   branches based on compares are delayed until the next loop.

   So we must precondition some registers and condition codes so that
   we don't exit the loop early on the first iteration.  */
	lbz	rWORD1,0(rSTR1)
	lbz	rWORD2,0(rSTR2)
	bdz	L(b11)
	cmplw	cr0,rWORD1,rWORD2
	lbz	rWORD3,1(rSTR1)
	lbz	rWORD4,1(rSTR2)
	bdz	L(b12)
	cmplw	cr1,rWORD3,rWORD4
	lbzu	rWORD5,2(rSTR1)
	lbzu	rWORD6,2(rSTR2)
	bdz	L(b13)
	.align	4
L(bLoop):
	lbzu	rWORD1,1(rSTR1)
	lbzu	rWORD2,1(rSTR2)
	bne	cr0,L(bLcr0)

	cmplw	cr6,rWORD5,rWORD6
	bdz	L(b3i)

	lbzu	rWORD3,1(rSTR1)
	lbzu	rWORD4,1(rSTR2)
	bne	cr1,L(bLcr1)

	cmplw	cr0,rWORD1,rWORD2
	bdz	L(b2i)

	lbzu	rWORD5,1(rSTR1)
	lbzu	rWORD6,1(rSTR2)
	bne	cr6,L(bLcr6)

	cmplw	cr1,rWORD3,rWORD4
	bdnz	L(bLoop)

/* We speculatively loading bytes before we have tested the previous
   bytes.  But we must avoid overrunning the length (in the ctr) to
   prevent these speculative loads from causing a segfault.  In this
   case the loop will exit early (before the all pending bytes are
   tested.  In this case we must complete the pending operations
   before returning.  */
L(b1i):
	bne	cr0,L(bLcr0)
	bne	cr1,L(bLcr1)
	b	L(bx56)
	.align	4
L(b2i):
	bne	cr6,L(bLcr6)
	bne	cr0,L(bLcr0)
	b	L(bx34)
	.align	4
L(b3i):
	bne	cr1,L(bLcr1)
	bne	cr6,L(bLcr6)
	b	L(bx12)
	.align	4
L(bLcr0):
	li	rRTN,1
	bgtlr	cr0
	li	rRTN,-1
	blr
L(bLcr1):
	li	rRTN,1
	bgtlr	cr1
	li	rRTN,-1
	blr
L(bLcr6):
	li	rRTN,1
	bgtlr	cr6
	li	rRTN,-1
	blr

L(b13):
	bne	cr0,L(bx12)
	bne	cr1,L(bx34)
L(bx56):
	sub	rRTN,rWORD5,rWORD6
	blr
	nop
L(b12):
	bne	cr0,L(bx12)
L(bx34):
	sub	rRTN,rWORD3,rWORD4
	blr

L(b11):
L(bx12):
	sub	rRTN,rWORD1,rWORD2
	blr

	.align	4
L(zeroLengthReturn):

L(zeroLength):
	li	rRTN,0
	blr

	cfi_adjust_cfa_offset(64)
	.align	4
/* At this point we know the strings have different alignment and the
   compare length is at least 8 bytes.  rBITDIF contains the low order
   2 bits of rSTR1 and cr5 contains the result of the logical compare
   of rBITDIF to 0.  If rBITDIF == 0 then rStr1 is word aligned and can
   perform the Wunaligned loop.

   Otherwise we know that rSTR1 is not aready word aligned yet.
   So we can force the string addresses to the next lower word
   boundary and special case this first word using shift left to
   eliminate bits preceeding the first byte.  Since we want to join the
   normal (Wualigned) compare loop, starting at the second word,
   we need to adjust the length (rN) and special case the loop
   versioning for the first W. This insures that the loop count is
   correct and the first W (shifted) is in the expected resister pair.  */
#define rSHL		r29	/* Unaligned shift left count.  */
#define rSHR		r28	/* Unaligned shift right count.  */
#define rB		r27	/* Left rotation temp for rWORD2.  */
#define rD		r26	/* Left rotation temp for rWORD4.  */
#define rF		r25	/* Left rotation temp for rWORD6.  */
#define rH		r24	/* Left rotation temp for rWORD8.  */
#define rA		r0	/* Right rotation temp for rWORD2.  */
#define rC		r12	/* Right rotation temp for rWORD4.  */
#define rE		r0	/* Right rotation temp for rWORD6.  */
#define rG		r12	/* Right rotation temp for rWORD8.  */
L(unaligned):
	stw	r29,40(r1)
	cfi_offset(r29,(40-64))
	clrlwi	rSHL,rSTR2,30
	stw	r28,36(r1)
	cfi_offset(r28,(36-64))
	beq	cr5,L(Wunaligned)
	stw	r27,32(r1)
	cfi_offset(r27,(32-64))
/* Adjust the logical start of rSTR2 to compensate for the extra bits
   in the 1st rSTR1 W.  */
	sub	r27,rSTR2,rBITDIF
/* But do not attempt to address the W before that W that contains
   the actual start of rSTR2.  */
	clrrwi	rSTR2,rSTR2,2
	stw	r26,28(r1)
	cfi_offset(r26,(28-64))
/* Compute the left/right shift counts for the unalign rSTR2,
   compensating for the logical (W aligned) start of rSTR1.  */
	clrlwi	rSHL,r27,30
	clrrwi	rSTR1,rSTR1,2
	stw	r25,24(r1)
	cfi_offset(r25,(24-64))
	slwi	rSHL,rSHL,3
	cmplw	cr5,r27,rSTR2
	add	rN,rN,rBITDIF
	slwi	r11,rBITDIF,3
	stw	r24,20(r1)
	cfi_offset(r24,(20-64))
	subfic	rSHR,rSHL,32
	srwi	rTMP,rN,4	/* Divide by 16 */
	andi.	rBITDIF,rN,12	/* Get the W remainder */
/* We normally need to load 2 Ws to start the unaligned rSTR2, but in
   this special case those bits may be discarded anyway.  Also we
   must avoid loading a W where none of the bits are part of rSTR2 as
   this may cross a page boundary and cause a page fault.  */
	li	rWORD8,0
	blt	cr5,L(dus0)
	lwz	rWORD8,0(rSTR2)
	la	rSTR2,4(rSTR2)
	slw	rWORD8,rWORD8,rSHL

L(dus0):
	lwz	rWORD1,0(rSTR1)
	lwz	rWORD2,0(rSTR2)
	cmplwi	cr1,rBITDIF,8
	cmplwi	cr7,rN,16
	srw	rG,rWORD2,rSHR
	clrlwi	rN,rN,30
	beq	L(duPs4)
	mtctr	rTMP
	or	rWORD8,rG,rWORD8
	bgt	cr1,L(duPs3)
	beq	cr1,L(duPs2)

/* Remainder is 4 */
	.align	4
L(dusP1):
	slw	rB,rWORD2,rSHL
	slw	rWORD7,rWORD1,r11
	slw	rWORD8,rWORD8,r11
	bge	cr7,L(duP1e)
/* At this point we exit early with the first word compare
   complete and remainder of 0 to 3 bytes.  See L(du14) for details on
   how we handle the remaining bytes.  */
	cmplw	cr5,rWORD7,rWORD8
	slwi.	rN,rN,3
	bne	cr5,L(duLcr5)
	cmplw	cr7,rN,rSHR
	beq	L(duZeroReturn)
	li	rA,0
	ble	cr7,L(dutrim)
	lwz	rWORD2,4(rSTR2)
	srw	rA,rWORD2,rSHR
	b	L(dutrim)
/* Remainder is 8 */
	.align	4
L(duPs2):
	slw	rH,rWORD2,rSHL
	slw	rWORD5,rWORD1,r11
	slw	rWORD6,rWORD8,r11
	b	L(duP2e)
/* Remainder is 12 */
	.align	4
L(duPs3):
	slw	rF,rWORD2,rSHL
	slw	rWORD3,rWORD1,r11
	slw	rWORD4,rWORD8,r11
	b	L(duP3e)
/* Count is a multiple of 16, remainder is 0 */
	.align	4
L(duPs4):
	mtctr	rTMP
	or	rWORD8,rG,rWORD8
	slw	rD,rWORD2,rSHL
	slw	rWORD1,rWORD1,r11
	slw	rWORD2,rWORD8,r11
	b	L(duP4e)

/* At this point we know rSTR1 is word aligned and the
   compare length is at least 8 bytes.  */
	.align	4
L(Wunaligned):
	stw	r27,32(r1)
	cfi_offset(r27,(32-64))
	clrrwi	rSTR2,rSTR2,2
	stw	r26,28(r1)
	cfi_offset(r26,(28-64))
	srwi	rTMP,rN,4	/* Divide by 16 */
	stw	r25,24(r1)
	cfi_offset(r25,(24-64))
	andi.	rBITDIF,rN,12	/* Get the W remainder */
	stw	r24,20(r1)
	cfi_offset(r24,(24-64))
	slwi	rSHL,rSHL,3
	lwz	rWORD6,0(rSTR2)
	lwzu	rWORD8,4(rSTR2)
	cmplwi	cr1,rBITDIF,8
	cmplwi	cr7,rN,16
	clrlwi	rN,rN,30
	subfic	rSHR,rSHL,32
	slw	rH,rWORD6,rSHL
	beq	L(duP4)
	mtctr	rTMP
	bgt	cr1,L(duP3)
	beq	cr1,L(duP2)

/* Remainder is 4 */
	.align	4
L(duP1):
	srw	rG,rWORD8,rSHR
	lwz	rWORD7,0(rSTR1)
	slw	rB,rWORD8,rSHL
	or	rWORD8,rG,rH
	blt	cr7,L(duP1x)
L(duP1e):
	lwz	rWORD1,4(rSTR1)
	lwz	rWORD2,4(rSTR2)
	cmplw	cr5,rWORD7,rWORD8
	srw	rA,rWORD2,rSHR
	slw	rD,rWORD2,rSHL
	or	rWORD2,rA,rB
	lwz	rWORD3,8(rSTR1)
	lwz	rWORD4,8(rSTR2)
	cmplw	cr0,rWORD1,rWORD2
	srw	rC,rWORD4,rSHR
	slw	rF,rWORD4,rSHL
	bne	cr5,L(duLcr5)
	or	rWORD4,rC,rD
	lwz	rWORD5,12(rSTR1)
	lwz	rWORD6,12(rSTR2)
	cmplw	cr1,rWORD3,rWORD4
	srw	rE,rWORD6,rSHR
	slw	rH,rWORD6,rSHL
	bne	cr0,L(duLcr0)
	or	rWORD6,rE,rF
	cmplw	cr6,rWORD5,rWORD6
	b	L(duLoop3)
	.align	4
/* At this point we exit early with the first word compare
   complete and remainder of 0 to 3 bytes.  See L(du14) for details on
   how we handle the remaining bytes.  */
L(duP1x):
	cmplw	cr5,rWORD7,rWORD8
	slwi.	rN,rN,3
	bne	cr5,L(duLcr5)
	cmplw	cr7,rN,rSHR
	beq	L(duZeroReturn)
	li	rA,0
	ble	cr7,L(dutrim)
	ld	rWORD2,8(rSTR2)
	srw	rA,rWORD2,rSHR
	b	L(dutrim)
/* Remainder is 8 */
	.align	4
L(duP2):
	srw	rE,rWORD8,rSHR
	lwz	rWORD5,0(rSTR1)
	or	rWORD6,rE,rH
	slw	rH,rWORD8,rSHL
L(duP2e):
	lwz	rWORD7,4(rSTR1)
	lwz	rWORD8,4(rSTR2)
	cmplw	cr6,rWORD5,rWORD6
	srw	rG,rWORD8,rSHR
	slw	rB,rWORD8,rSHL
	or	rWORD8,rG,rH
	blt	cr7,L(duP2x)
	lwz	rWORD1,8(rSTR1)
	lwz	rWORD2,8(rSTR2)
	cmplw	cr5,rWORD7,rWORD8
	bne	cr6,L(duLcr6)
	srw	rA,rWORD2,rSHR
	slw	rD,rWORD2,rSHL
	or	rWORD2,rA,rB
	lwz	rWORD3,12(rSTR1)
	lwz	rWORD4,12(rSTR2)
	cmplw	cr0,rWORD1,rWORD2
	bne	cr5,L(duLcr5)
	srw	rC,rWORD4,rSHR
	slw	rF,rWORD4,rSHL
	or	rWORD4,rC,rD
	addi	rSTR1,rSTR1,4
	addi	rSTR2,rSTR2,4
	cmplw	cr1,rWORD3,rWORD4
	b	L(duLoop2)
	.align	4
L(duP2x):
	cmplw	cr5,rWORD7,rWORD8
	addi	rSTR1,rSTR1,4
	addi	rSTR2,rSTR2,4
	bne	cr6,L(duLcr6)
	slwi.	rN,rN,3
	bne	cr5,L(duLcr5)
	cmplw	cr7,rN,rSHR
	beq	L(duZeroReturn)
	li	rA,0
	ble	cr7,L(dutrim)
	lwz	rWORD2,4(rSTR2)
	srw	rA,rWORD2,rSHR
	b	L(dutrim)

/* Remainder is 12 */
	.align	4
L(duP3):
	srw	rC,rWORD8,rSHR
	lwz	rWORD3,0(rSTR1)
	slw	rF,rWORD8,rSHL
	or	rWORD4,rC,rH
L(duP3e):
	lwz	rWORD5,4(rSTR1)
	lwz	rWORD6,4(rSTR2)
	cmplw	cr1,rWORD3,rWORD4
	srw	rE,rWORD6,rSHR
	slw	rH,rWORD6,rSHL
	or	rWORD6,rE,rF
	lwz	rWORD7,8(rSTR1)
	lwz	rWORD8,8(rSTR2)
	cmplw	cr6,rWORD5,rWORD6
	bne	cr1,L(duLcr1)
	srw	rG,rWORD8,rSHR
	slw	rB,rWORD8,rSHL
	or	rWORD8,rG,rH
	blt	cr7,L(duP3x)
	lwz	rWORD1,12(rSTR1)
	lwz	rWORD2,12(rSTR2)
	cmplw	cr5,rWORD7,rWORD8
	bne	cr6,L(duLcr6)
	srw	rA,rWORD2,rSHR
	slw	rD,rWORD2,rSHL
	or	rWORD2,rA,rB
	addi	rSTR1,rSTR1,8
	addi	rSTR2,rSTR2,8
	cmplw	cr0,rWORD1,rWORD2
	b	L(duLoop1)
	.align	4
L(duP3x):
	addi	rSTR1,rSTR1,8
	addi	rSTR2,rSTR2,8
	bne	cr1,L(duLcr1)
	cmplw	cr5,rWORD7,rWORD8
	bne	cr6,L(duLcr6)
	slwi.	rN,rN,3
	bne	cr5,L(duLcr5)
	cmplw	cr7,rN,rSHR
	beq	L(duZeroReturn)
	li	rA,0
	ble	cr7,L(dutrim)
	lwz	rWORD2,4(rSTR2)
	srw	rA,rWORD2,rSHR
	b	L(dutrim)

/* Count is a multiple of 16, remainder is 0 */
	.align	4
L(duP4):
	mtctr	rTMP
	srw	rA,rWORD8,rSHR
	lwz	rWORD1,0(rSTR1)
	slw	rD,rWORD8,rSHL
	or	rWORD2,rA,rH
L(duP4e):
	lwz	rWORD3,4(rSTR1)
	lwz	rWORD4,4(rSTR2)
	cmplw	cr0,rWORD1,rWORD2
	srw	rC,rWORD4,rSHR
	slw	rF,rWORD4,rSHL
	or	rWORD4,rC,rD
	lwz	rWORD5,8(rSTR1)
	lwz	rWORD6,8(rSTR2)
	cmplw	cr1,rWORD3,rWORD4
	bne	cr0,L(duLcr0)
	srw	rE,rWORD6,rSHR
	slw	rH,rWORD6,rSHL
	or	rWORD6,rE,rF
	lwzu	rWORD7,12(rSTR1)
	lwzu	rWORD8,12(rSTR2)
	cmplw	cr6,rWORD5,rWORD6
	bne	cr1,L(duLcr1)
	srw	rG,rWORD8,rSHR
	slw	rB,rWORD8,rSHL
	or	rWORD8,rG,rH
	cmplw	cr5,rWORD7,rWORD8
	bdz	L(du24)		/* Adjust CTR as we start with +4 */
/* This is the primary loop */
	.align	4
L(duLoop):
	lwz	rWORD1,4(rSTR1)
	lwz	rWORD2,4(rSTR2)
	cmplw	cr1,rWORD3,rWORD4
	bne	cr6,L(duLcr6)
	srw	rA,rWORD2,rSHR
	slw	rD,rWORD2,rSHL
	or	rWORD2,rA,rB
L(duLoop1):
	lwz	rWORD3,8(rSTR1)
	lwz	rWORD4,8(rSTR2)
	cmplw	cr6,rWORD5,rWORD6
	bne	cr5,L(duLcr5)
	srw	rC,rWORD4,rSHR
	slw	rF,rWORD4,rSHL
	or	rWORD4,rC,rD
L(duLoop2):
	lwz	rWORD5,12(rSTR1)
	lwz	rWORD6,12(rSTR2)
	cmplw	cr5,rWORD7,rWORD8
	bne	cr0,L(duLcr0)
	srw	rE,rWORD6,rSHR
	slw	rH,rWORD6,rSHL
	or	rWORD6,rE,rF
L(duLoop3):
	lwzu	rWORD7,16(rSTR1)
	lwzu	rWORD8,16(rSTR2)
	cmplw	cr0,rWORD1,rWORD2
	bne	cr1,L(duLcr1)
	srw	rG,rWORD8,rSHR
	slw	rB,rWORD8,rSHL
	or	rWORD8,rG,rH
	bdnz	L(duLoop)

L(duL4):
	bne	cr1,L(duLcr1)
	cmplw	cr1,rWORD3,rWORD4
	bne	cr6,L(duLcr6)
	cmplw	cr6,rWORD5,rWORD6
	bne	cr5,L(duLcr5)
	cmplw	cr5,rWORD7,rWORD8
L(du44):
	bne	cr0,L(duLcr0)
L(du34):
	bne	cr1,L(duLcr1)
L(du24):
	bne	cr6,L(duLcr6)
L(du14):
	slwi.	rN,rN,3
	bne	cr5,L(duLcr5)
/* At this point we have a remainder of 1 to 3 bytes to compare.  We use
   shift right to eliminate bits beyond the compare length.

   However it may not be safe to load rWORD2 which may be beyond the
   string length. So we compare the bit length of the remainder to
   the right shift count (rSHR). If the bit count is less than or equal
   we do not need to load rWORD2 (all significant bits are already in
   rB).  */
	cmplw	cr7,rN,rSHR
	beq	L(duZeroReturn)
	li	rA,0
	ble	cr7,L(dutrim)
	lwz	rWORD2,4(rSTR2)
	srw	rA,rWORD2,rSHR
	.align	4
L(dutrim):
	lwz	rWORD1,4(rSTR1)
	lwz	r31,48(1)
	subfic	rN,rN,32	/* Shift count is 32 - (rN * 8).  */
	or	rWORD2,rA,rB
	lwz	r30,44(1)
	lwz	r29,40(r1)
	srw	rWORD1,rWORD1,rN
	srw	rWORD2,rWORD2,rN
	lwz	r28,36(r1)
	lwz	r27,32(r1)
	cmplw	rWORD1,rWORD2
	li	rRTN,0
	beq	L(dureturn26)
	li	rRTN,1
	bgt	L(dureturn26)
	li	rRTN,-1
	b	L(dureturn26)
	.align	4
L(duLcr0):
	lwz	r31,48(1)
	lwz	r30,44(1)
	li	rRTN,1
	bgt	cr0,L(dureturn29)
	lwz	r29,40(r1)
	lwz	r28,36(r1)
	li	rRTN,-1
	b	L(dureturn27)
	.align	4
L(duLcr1):
	lwz	r31,48(1)
	lwz	r30,44(1)
	li	rRTN,1
	bgt	cr1,L(dureturn29)
	lwz	r29,40(r1)
	lwz	r28,36(r1)
	li	rRTN,-1
	b	L(dureturn27)
	.align	4
L(duLcr6):
	lwz	r31,48(1)
	lwz	r30,44(1)
	li	rRTN,1
	bgt	cr6,L(dureturn29)
	lwz	r29,40(r1)
	lwz	r28,36(r1)
	li	rRTN,-1
	b	L(dureturn27)
	.align	4
L(duLcr5):
	lwz	r31,48(1)
	lwz	r30,44(1)
	li	rRTN,1
	bgt	cr5,L(dureturn29)
	lwz	r29,40(r1)
	lwz	r28,36(r1)
	li	rRTN,-1
	b	L(dureturn27)
	.align	3
L(duZeroReturn):
	li	rRTN,0
	.align	4
L(dureturn):
	lwz	r31,48(1)
	lwz	r30,44(1)
L(dureturn29):
	lwz	r29,40(r1)
	lwz	r28,36(r1)
L(dureturn27):
	lwz	r27,32(r1)
L(dureturn26):
	lwz	r26,28(r1)
L(dureturn25):
	lwz	r25,24(r1)
	lwz	r24,20(r1)
	lwz	1,0(1)
	blr
END (BP_SYM (memcmp))
libc_hidden_builtin_def (memcmp)
weak_alias (memcmp,bcmp)
