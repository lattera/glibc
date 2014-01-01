/* Optimized strcpy/stpcpy implementation for PowerPC64/POWER7.
   Copyright (C) 2013-2014 Free Software Foundation, Inc.
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

/* Implements the function

   char * [r3] strcpy (char *dest [r3], const char *src [r4])

   or

   char * [r3] strcpy (char *dest [r3], const char *src [r4])

   if USE_AS_STPCPY is defined. It tries to use aligned memory accesses
   when possible using the following algorithm:

   if (((((uintptr_t)dst & 0x7UL) == 0) && ((uintptr_t)src & 0x7UL) == 0))
     goto aligned_doubleword_copy;
   if (((((uintptr_t)dst & 0x3UL) == 0) && ((uintptr_t)src & 0x3UL) == 0))
     goto aligned_word_copy;
   if (((uintptr_t)dst & 0x7UL) == ((uintptr_t)src & 0x7UL))
     goto same_alignment;
   goto unaligned;

   The aligned comparison are made using cmpb instructions.  */

#ifdef USE_AS_STPCPY
# define FUNC_NAME __stpcpy
#else
# define FUNC_NAME strcpy
#endif

	.machine  power7
EALIGN (FUNC_NAME, 4, 0)
	CALL_MCOUNT 2

#define rTMP	r0
#ifdef USE_AS_STPCPY
#define rRTN	r3	/* pointer to previous word/doubleword in dest */
#else
#define rRTN	r12	/* pointer to previous word/doubleword in dest */
#endif
#define rSRC	r4	/* pointer to previous word/doubleword in src */
#define rMASK	r5	/* mask 0xffffffff | 0xffffffffffffffff */
#define rWORD	r6	/* current word from src */
#define rALT	r7	/* alternate word from src */
#define rRTNAL	r8	/* alignment of return pointer */
#define rSRCAL	r9	/* alignment of source pointer */
#define rALCNT	r10	/* bytes to read to reach 8 bytes alignment */
#define rSUBAL	r11	/* doubleword minus unaligned displacement */

#ifndef USE_AS_STPCPY
/* Save the dst pointer to use as return value.  */
	mr	rRTN, r3
#endif
	or	rTMP, rSRC, rRTN
	clrldi.	rTMP, rTMP, 61
	bne	L(check_word_alignment)
	b	L(aligned_doubleword_copy)

L(same_alignment):
/* Src and dst with same alignment: align both to doubleword.  */
	mr	rALCNT, rRTN
	lbz	rWORD, 0(rSRC)
	subfic	rSUBAL, rRTNAL, 8
	addi	rRTN, rRTN, 1
	addi	rSRC, rSRC, 1
	cmpdi	cr7, rWORD, 0
	stb	rWORD, 0(rALCNT)
	beq	cr7, L(s2)

	add	rALCNT, rALCNT, rSUBAL
	subf	rALCNT, rRTN, rALCNT
	addi	rALCNT, rALCNT, 1
	mtctr	rALCNT
	b	L(s1)

	.align 4
L(s0):
	addi	rSRC, rSRC, 1
	lbz	rWORD, -1(rSRC)
	cmpdi	cr7, rWORD, 0
	stb	rWORD, -1(rALCNT)
	beqlr	cr7
	mr	rRTN, rALCNT
L(s1):
	addi	rALCNT, rRTN,1
	bdnz	L(s0)
	b L(aligned_doubleword_copy)
	.align 4
L(s2):
	mr	rRTN, rALCNT
	blr

/* For doubleword aligned memory, operate using doubleword load and stores.  */
	.align 4
L(aligned_doubleword_copy):
	li	rMASK, 0
	addi	rRTN, rRTN, -8
	ld	rWORD, 0(rSRC)
	b	L(g2)

	.align 4
L(g0):	ldu	rALT, 8(rSRC)
	stdu	rWORD, 8(rRTN)
	cmpb	rTMP, rALT, rMASK
	cmpdi	rTMP, 0
	bne	L(g1)
	ldu	rWORD, 8(rSRC)
	stdu	rALT, 8(rRTN)
L(g2):	cmpb	rTMP, rWORD, rMASK
	cmpdi	rTMP, 0		/* If rTMP is 0, no null's have been found.  */
	beq	L(g0)

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
	beqlr
	extrdi.	rTMP, rALT, 8, 8
	stbu	rTMP, 1(rRTN)
	beqlr
	extrdi.	rTMP, rALT, 8, 16
	stbu	rTMP, 1(rRTN)
	beqlr
	extrdi.	rTMP, rALT, 8, 24
	stbu	rTMP, 1(rRTN)
	beqlr
	extrdi.	rTMP, rALT, 8, 32
	stbu	rTMP, 1(rRTN)
	beqlr
	extrdi.	rTMP, rALT, 8, 40
	stbu	rTMP, 1(rRTN)
	beqlr
	extrdi.	rTMP, rALT, 8, 48
	stbu	rTMP, 1(rRTN)
	beqlr
	stbu	rALT, 1(rRTN)
#endif
	blr

L(check_word_alignment):
	clrldi. rTMP, rTMP, 62
	beq	L(aligned_word_copy)
	rldicl	rRTNAL, rRTN, 0, 61
	rldicl	rSRCAL, rSRC, 0, 61
	cmpld	cr7, rSRCAL, rRTNAL
	beq	cr7, L(same_alignment)
	b	L(unaligned)

/* For word aligned memory, operate using word load and stores.  */
	.align	4
L(aligned_word_copy):
	li	rMASK, 0
	addi	rRTN, rRTN, -4
	lwz	rWORD, 0(rSRC)
	b	L(g5)

	.align	4
L(g3):	lwzu	rALT, 4(rSRC)
	stwu	rWORD, 4(rRTN)
	cmpb	rTMP, rALT, rMASK
	cmpwi	rTMP, 0
	bne	L(g4)
	lwzu	rWORD, 4(rSRC)
	stwu	rALT, 4(rRTN)
L(g5):	cmpb	rTMP, rWORD, rMASK
	cmpwi	rTMP, 0		/* If rTMP is 0, no null in word.  */
	beq	L(g3)

	mr      rALT, rWORD
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
	rlwinm. rTMP, rALT, 8, 24, 31
	stbu    rTMP, 4(rRTN)
	beqlr
	rlwinm. rTMP, rALT, 16, 24, 31
	stbu    rTMP, 1(rRTN)
	beqlr
	rlwinm. rTMP, rALT, 24, 24, 31
	stbu    rTMP, 1(rRTN)
	beqlr
	stbu    rALT, 1(rRTN)
#endif
	blr

/* Oh well.  In this case, we just do a byte-by-byte copy.  */
	.align	4
L(unaligned):
	lbz	rWORD, 0(rSRC)
	addi	rRTN, rRTN, -1
	cmpdi	rWORD, 0
	beq	L(u2)

	.align 	5
L(u0):	lbzu	rALT, 1(rSRC)
	stbu	rWORD, 1(rRTN)
	cmpdi	rALT, 0
	beq	L(u1)
	lbzu	rWORD, 1(rSRC)
	stbu	rALT, 1(rRTN)
	cmpdi	rWORD, 0
	beq	L(u2)
	lbzu	rALT, 1(rSRC)
	stbu	rWORD, 1(rRTN)
	cmpdi	rALT, 0
	beq	L(u1)
	lbzu	rWORD, 1(rSRC)
	stbu	rALT, 1(rRTN)
	cmpdi	rWORD, 0
	bne	L(u0)
L(u2):	stbu	rWORD, 1(rRTN)
	blr
L(u1):	stbu	rALT, 1(rRTN)
	blr
END (FUNC_NAME)

#ifndef USE_AS_STPCPY
libc_hidden_builtin_def (strcpy)
#endif
