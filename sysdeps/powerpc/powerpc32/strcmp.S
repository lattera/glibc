/* Optimized strcmp implementation for PowerPC.
   Copyright (C) 1997, 1999, 2000, 2003 Free Software Foundation, Inc.
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

/* See strlen.s for comments on how the end-of-string testing works.  */

/* int [r3] strcmp (const char *s1 [r3], const char *s2 [r4])  */

EALIGN (BP_SYM (strcmp), 4, 0)

#define rTMP	r0
#define rRTN	r3
#define rSTR1	r3	/* first string arg */
#define rSTR2	r4	/* second string arg */
#if __BOUNDED_POINTERS__
# define rHIGH1	r11
# define rHIGH2 r12
#endif
#define rWORD1	r5	/* current word in s1 */
#define rWORD2	r6	/* current word in s2 */
#define rFEFE	r7	/* constant 0xfefefeff (-0x01010101) */
#define r7F7F	r8	/* constant 0x7f7f7f7f */
#define rNEG	r9	/* ~(word in s1 | 0x7f7f7f7f) */
#define rBITDIF	r10	/* bits that differ in s1 & s2 words */

	CHECK_BOUNDS_LOW (rSTR1, rTMP, rHIGH1)
	CHECK_BOUNDS_LOW (rSTR2, rTMP, rHIGH2)

	or	rTMP, rSTR2, rSTR1
	clrlwi.	rTMP, rTMP, 30
	lis	rFEFE, -0x101
	bne	L(unaligned)

	lwz	rWORD1, 0(rSTR1)
	lwz	rWORD2, 0(rSTR2)
	lis	r7F7F, 0x7f7f
	addi	rFEFE, rFEFE, -0x101
	addi	r7F7F, r7F7F, 0x7f7f
	b	L(g1)

L(g0):	lwzu	rWORD1, 4(rSTR1)
	bne	cr1, L(different)
	lwzu	rWORD2, 4(rSTR2)
L(g1):	add	rTMP, rFEFE, rWORD1
	nor	rNEG, r7F7F, rWORD1
	and.	rTMP, rTMP, rNEG
	cmpw	cr1, rWORD1, rWORD2
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
	cntlzw	rBITDIF, rBITDIF
	cntlzw	rNEG, rNEG
	addi	rNEG, rNEG, 7
	cmpw	cr1, rNEG, rBITDIF
	sub	rRTN, rWORD1, rWORD2
	bgelr+	cr1
L(equal):
	li	rRTN, 0
	/* GKM FIXME: check high bounds.  */
	blr

L(different):
	lwz	rWORD1, -4(rSTR1)
	xor.	rBITDIF, rWORD1, rWORD2
	sub	rRTN, rWORD1, rWORD2
	bgelr+
L(highbit):
	ori	rRTN, rWORD2, 1
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
	cmpw	rWORD1, rWORD2
	bne-	L(u3)
	lbzu	rWORD1, 1(rSTR1)
	lbzu	rWORD2, 1(rSTR2)
	cmpwi	cr1, rWORD1, 0
	cmpw	rWORD1, rWORD2
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
