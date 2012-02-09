/* Optimized strcmp implementation for PowerPC64.
   Copyright (C) 1997, 1999, 2000, 2002, 2003, 2011 Free Software Foundation, Inc.
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

/* See strlen.s for comments on how the end-of-string testing works.  */

/* int [r3] strcmp (const char *s1 [r3], const char *s2 [r4])  */

EALIGN (BP_SYM(strcmp), 4, 0)
	CALL_MCOUNT 2

#define rTMP	r0
#define rRTN	r3
#define rSTR1	r3	/* first string arg */
#define rSTR2	r4	/* second string arg */
/* Note:  The Bounded pointer support in this code is broken.  This code
   was inherited from PPC32 and that support was never completed.
   Current PPC gcc does not support -fbounds-check or -fbounded-pointers.
   These artifacts are left in the code as a reminder in case we need
   bounded pointer support in the future.  */
#if __BOUNDED_POINTERS__
# define rHIGH1	r11
# define rHIGH2 r12
#endif
#define rWORD1	r5	/* current word in s1 */
#define rWORD2	r6	/* current word in s2 */
#define rFEFE	r7	/* constant 0xfefefefefefefeff (-0x0101010101010101) */
#define r7F7F	r8	/* constant 0x7f7f7f7f7f7f7f7f */
#define rNEG	r9	/* ~(word in s1 | 0x7f7f7f7f7f7f7f7f) */
#define rBITDIF	r10	/* bits that differ in s1 & s2 words */

	CHECK_BOUNDS_LOW (rSTR1, rTMP, rHIGH1)
	CHECK_BOUNDS_LOW (rSTR2, rTMP, rHIGH2)

	dcbt	0,rSTR1
	or	rTMP, rSTR2, rSTR1
	dcbt	0,rSTR2
	clrldi.	rTMP, rTMP, 61
	lis	rFEFE, -0x101
	bne	L(unaligned)

	ld	rWORD1, 0(rSTR1)
	ld	rWORD2, 0(rSTR2)
	lis	r7F7F, 0x7f7f
	addi	rFEFE, rFEFE, -0x101
	addi	r7F7F, r7F7F, 0x7f7f
	sldi	rTMP, rFEFE, 32
	insrdi	r7F7F, r7F7F, 32, 0
	add	rFEFE, rFEFE, rTMP
	b	L(g1)

L(g0):	ldu	rWORD1, 8(rSTR1)
	bne	cr1, L(different)
	ldu	rWORD2, 8(rSTR2)
L(g1):	add	rTMP, rFEFE, rWORD1
	nor	rNEG, r7F7F, rWORD1

	and.	rTMP, rTMP, rNEG
	cmpd	cr1, rWORD1, rWORD2
	beq+	L(g0)
L(endstring):
/* OK. We've hit the end of the string. We need to be careful that
   we don't compare two strings as different because of gunk beyond
   the end of the strings...  */
	and	rTMP, r7F7F, rWORD1
	beq	cr1, L(equal)
	add	rTMP, rTMP, r7F7F
	xor.	rBITDIF, rWORD1, rWORD2

	andc	rNEG, rNEG, rTMP
	blt-	L(highbit)
	cntlzd	rBITDIF, rBITDIF
	cntlzd	rNEG, rNEG
	addi	rNEG, rNEG, 7
	cmpd	cr1, rNEG, rBITDIF
	sub	rRTN, rWORD1, rWORD2
	blt-	cr1, L(equal)
	sradi	rRTN, rRTN, 63
	ori	rRTN, rRTN, 1
	blr
L(equal):
	li	rRTN, 0
	/* GKM FIXME: check high bounds.  */
	blr

L(different):
	ld	rWORD1, -8(rSTR1)
	xor.	rBITDIF, rWORD1, rWORD2
	sub	rRTN, rWORD1, rWORD2
	blt-	L(highbit)
	sradi	rRTN, rRTN, 63
	ori	rRTN, rRTN, 1
	blr
L(highbit):
	srdi	rWORD2, rWORD2, 56
	srdi	rWORD1, rWORD1, 56
	sub	rRTN, rWORD1, rWORD2
	/* GKM FIXME: check high bounds.  */
	blr


/* Oh well.  In this case, we just do a byte-by-byte comparison.  */
	.align 4
L(unaligned):
	lbz	rWORD1, 0(rSTR1)
	lbz	rWORD2, 0(rSTR2)
	b	L(u1)

L(u0):	lbzu	rWORD1, 1(rSTR1)
	bne-	L(u4)
	lbzu	rWORD2, 1(rSTR2)
L(u1):	cmpwi	cr1, rWORD1, 0
	beq-	cr1, L(u3)
	cmpd	rWORD1, rWORD2
	bne-	L(u3)
	lbzu	rWORD1, 1(rSTR1)
	lbzu	rWORD2, 1(rSTR2)
	cmpdi	cr1, rWORD1, 0
	cmpd	rWORD1, rWORD2
	bne+	cr1, L(u0)
L(u3):	sub	rRTN, rWORD1, rWORD2
	/* GKM FIXME: check high bounds.  */
	blr
L(u4):	lbz	rWORD1, -1(rSTR1)
	sub	rRTN, rWORD1, rWORD2
	/* GKM FIXME: check high bounds.  */
	blr
END (BP_SYM (strcmp))
libc_hidden_builtin_def (strcmp)
