/* Optimized strcpy implementation for PowerPC64.
   Copyright (C) 1997-2015 Free Software Foundation, Inc.
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

/* See strlen.s for comments on how the end-of-string testing works.  */

/* char * [r3] strcpy (char *dest [r3], const char *src [r4])  */

#ifdef USE_AS_STPCPY
# define FUNC_NAME __stpcpy
#else
# define FUNC_NAME strcpy
#endif

EALIGN (FUNC_NAME, 4, 0)
	CALL_MCOUNT 2

#define rTMP	r0
#ifdef USE_AS_STPCPY
#define rRTN    r3      /* pointer to previous word/doubleword in dest */
#else
#define rRTN    r12     /* pointer to previous word/doubleword in dest */
#endif
#define rSRC	r4	/* pointer to previous word/doubleword in src */
#define rWORD	r6	/* current word from src */
#define rFEFE	r7	/* constant 0xfefefeff | 0xfefefefefefefeff */
#define r7F7F	r8	/* constant 0x7f7f7f7f | 0x7f7f7f7f7f7f7f7f */
#define rNEG	r9	/* ~(word in s1 | r7F7F) */
#define rALT	r10	/* alternate word from src */

#ifndef USE_AS_STPCPY
/* Save the dst pointer to use as return value.  */
	mr      rRTN, r3
#endif
	or	rTMP, rSRC, rRTN
	clrldi.	rTMP, rTMP, 61
	bne	L(check_word_alignment)

/* For doubleword aligned memory, operate using doubleword load and stores.  */
	addi	rRTN, rRTN, -8

	lis	rFEFE, -0x101
	lis	r7F7F, 0x7f7f
	ld	rWORD, 0(rSRC)
	addi	rFEFE, rFEFE, -0x101
	addi	r7F7F, r7F7F, 0x7f7f
	sldi	rTMP, rFEFE, 32
	insrdi	r7F7F, r7F7F, 32, 0
	add	rFEFE, rFEFE, rTMP
	b	L(g2)

L(g0):	ldu	rALT, 8(rSRC)
	stdu	rWORD, 8(rRTN)
	add	rTMP, rFEFE, rALT
	nor	rNEG, r7F7F, rALT
	and.	rTMP, rTMP, rNEG
	bne-	L(g1)
	ldu	rWORD, 8(rSRC)
	stdu	rALT, 8(rRTN)
L(g2):	add	rTMP, rFEFE, rWORD
	nor	rNEG, r7F7F, rWORD
	and.	rTMP, rTMP, rNEG
	beq+	L(g0)

	mr	rALT, rWORD
/* We've hit the end of the string.  Do the rest byte-by-byte.  */
L(g1):
#ifdef __LITTLE_ENDIAN__
	extrdi.	rTMP, rALT, 8, 56
	stbu	rALT, 8(rRTN)
	beqlr-
	extrdi.	rTMP, rALT, 8, 48
	stbu	rTMP, 1(rRTN)
	beqlr-
	extrdi.	rTMP, rALT, 8, 40
	stbu	rTMP, 1(rRTN)
	beqlr-
	extrdi.	rTMP, rALT, 8, 32
	stbu	rTMP, 1(rRTN)
	beqlr-
	extrdi.	rTMP, rALT, 8, 24
	stbu	rTMP, 1(rRTN)
	beqlr-
	extrdi.	rTMP, rALT, 8, 16
	stbu	rTMP, 1(rRTN)
	beqlr-
	extrdi.	rTMP, rALT, 8, 8
	stbu	rTMP, 1(rRTN)
	beqlr-
	extrdi	rTMP, rALT, 8, 0
	stbu	rTMP, 1(rRTN)
#else
	extrdi.	rTMP, rALT, 8, 0
	stbu	rTMP, 8(rRTN)
	beqlr-
	extrdi.	rTMP, rALT, 8, 8
	stbu	rTMP, 1(rRTN)
	beqlr-
	extrdi.	rTMP, rALT, 8, 16
	stbu	rTMP, 1(rRTN)
	beqlr-
	extrdi.	rTMP, rALT, 8, 24
	stbu	rTMP, 1(rRTN)
	beqlr-
	extrdi.	rTMP, rALT, 8, 32
	stbu	rTMP, 1(rRTN)
	beqlr
	extrdi.	rTMP, rALT, 8, 40
	stbu	rTMP, 1(rRTN)
	beqlr-
	extrdi.	rTMP, rALT, 8, 48
	stbu	rTMP, 1(rRTN)
	beqlr-
	stbu	rALT, 1(rRTN)
#endif
	blr

L(check_word_alignment):
	clrldi. rTMP, rTMP, 62
	bne     L(unaligned)

/* For word aligned memory, operate using word load and stores.  */
	addi	rRTN, rRTN, -4

	lis	rFEFE, -0x101
	lis	r7F7F, 0x7f7f
	lwz	rWORD, 0(rSRC)
	addi	rFEFE, rFEFE, -0x101
	addi	r7F7F, r7F7F, 0x7f7f
	b	L(g5)

L(g3):	lwzu	rALT, 4(rSRC)
	stwu	rWORD, 4(rRTN)
	add	rTMP, rFEFE, rALT
	nor	rNEG, r7F7F, rALT
	and.	rTMP, rTMP, rNEG
	bne-	L(g4)
	lwzu	rWORD, 4(rSRC)
	stwu	rALT, 4(rRTN)
L(g5):	add	rTMP, rFEFE, rWORD
	nor	rNEG, r7F7F, rWORD
	and.	rTMP, rTMP, rNEG
	beq+	L(g3)

	mr	rALT, rWORD
/* We've hit the end of the string.  Do the rest byte-by-byte.  */
L(g4):
#ifdef __LITTLE_ENDIAN__
	rlwinm.	rTMP, rALT, 0, 24, 31
	stbu	rALT, 4(rRTN)
	beqlr-
	rlwinm.	rTMP, rALT, 24, 24, 31
	stbu	rTMP, 1(rRTN)
	beqlr-
	rlwinm.	rTMP, rALT, 16, 24, 31
	stbu	rTMP, 1(rRTN)
	beqlr-
	rlwinm	rTMP, rALT, 8, 24, 31
	stbu	rTMP, 1(rRTN)
#else
	rlwinm.	rTMP, rALT, 8, 24, 31
	stbu	rTMP, 4(rRTN)
	beqlr-
	rlwinm.	rTMP, rALT, 16, 24, 31
	stbu	rTMP, 1(rRTN)
	beqlr-
	rlwinm.	rTMP, rALT, 24, 24, 31
	stbu	rTMP, 1(rRTN)
	beqlr-
	stbu	rALT, 1(rRTN)
#endif
	blr

/* Oh well.  In this case, we just do a byte-by-byte copy.  */
	.align 4
	nop
L(unaligned):
	lbz	rWORD, 0(rSRC)
	addi	rRTN, rRTN, -1
	cmpwi	rWORD, 0
	beq-	L(u2)

L(u0):	lbzu	rALT, 1(rSRC)
	stbu	rWORD, 1(rRTN)
	cmpwi	rALT, 0
	beq-	L(u1)
	nop		/* Let 601 load start of loop.  */
	lbzu	rWORD, 1(rSRC)
	stbu	rALT, 1(rRTN)
	cmpwi	rWORD, 0
	bne+	L(u0)
L(u2):	stbu	rWORD, 1(rRTN)
	blr
L(u1):	stbu	rALT, 1(rRTN)
	blr
END (FUNC_NAME)

#ifndef USE_AS_STPCPY
libc_hidden_builtin_def (strcpy)
#endif
