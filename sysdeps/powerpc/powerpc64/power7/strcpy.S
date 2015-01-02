/* Optimized strcpy/stpcpy implementation for PowerPC64/POWER7.
   Copyright (C) 2013-2015 Free Software Foundation, Inc.
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
	bne	L(check_alignment)
	b	L(aligned_doubleword_copy)

	.align 4
L(check_alignment):
	rldicl	rRTNAL, rRTN, 0, 61
	rldicl	rSRCAL, rSRC, 0, 61
	cmpld	cr7, rSRCAL, rRTNAL
	beq	cr7, L(same_alignment)
	b	L(unaligned)

	.align 4
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

	.align	4
L(unaligned):
	cmpdi	rSRCAL, 0		/* Check src alignment */
	beq	L(srcaligndstunalign)
	/* src is unaligned */
	rlwinm	r10, rSRC, 3,26,28	/* Calculate padding.  */
	clrrdi	rSRC, rSRC, 3		/* Align the addr to dw boundary */
	ld	rWORD, 0(rSRC)		/* Load doubleword from memory.  */
	li	rTMP, 0
	/* Discard bits not part of the string */
#ifdef __LITTLE_ENDIAN__
	srd	rALT, rWORD, r10
#else
	sld	rALT, rWORD, r10
#endif
	cmpb	rTMP, rALT, rTMP	/* Compare each byte against null */
	/* Discard bits not part of the string */
#ifdef __LITTLE_ENDIAN__
	sld	rTMP, rTMP, r10
#else
	srd	rTMP, rTMP, r10
#endif
	cmpdi	rTMP, 0
	bne	L(bytebybyte)		/* if it has null, copy byte by byte */
	subfic	r8, r9, 8
	rlwinm	r5, rRTN, 3,26,28	/* Calculate padding in bits.  */
	rldicl	r9, rRTN, 0, 61		/* Calculate padding in bytes. */
	addi	rRTN, rRTN, -1

	cmpdi	r5, 0			/* check dest alignment */
	beq	L(srcunaligndstalign)

	/* both src and dst unaligned */
#ifdef __LITTLE_ENDIAN__
	sld	rWORD, rALT, r10
	mr 	r11, r10
	addi	r11, r11, -8		/* Adjust byte pointer on loaded dw */
#else
	srd	rWORD, rALT, r10
	subfic	r11, r10, 64
#endif
	/* dst alignment is greater then src alignment? */
	cmpd	cr7, r5, r10
	blt	cr7, L(dst_align_small)
	/* src alignment is less than dst */

	/* Calculate the dst alignment differnce */
	subfic	rALT, r9, 8
	mtctr	rALT

	/* Write till dst is aligned */
	cmpdi	rTMP, rALT, 4
	blt	L(storebyte1)		/* less than 4, store byte by byte */
	beq	L(equal1)		/* if its 4, store word */
	addi	rTMP, rALT, -4		/* greater than 4, so stb and stw */
	mtctr	rTMP
L(storebyte1):
#ifdef __LITTLE_ENDIAN__
	addi	r11, r11, 8		/* Adjust byte pointer on loaded dw */
#else
	addi	r11, r11, -8
#endif
	srd	rALT, rWORD, r11
	stbu	rALT, 1(rRTN)
	bdnz	L(storebyte1)

	subfic	rALT, r9, 8		/* Check the remaining bytes */
	cmpdi	rTMP, rALT, 4
	blt	L(proceed)

	.align 4
L(equal1):
#ifdef __LITTLE_ENDIAN__
	addi	r11, r11, 8		/* Adjust byte pointer on loaded dw */
	srd	rALT, rWORD, r11
#else
	subfic	r11, r11, 64
	sld	rALT, rWORD, r11
	srdi	rALT, rALT, 32
#endif
	stw	rALT, 1(rRTN)
	addi	rRTN, rRTN, 4

L(proceed):
	mr	rALT, rWORD
	/* calculate the Left over bytes to be written */
	subfic	r11, r10, 64
	subfic	r5, r5, 64
	subf	r5, r5, r11		/* remaining bytes on second dw */
        subfic	r10, r5, 64		/* remaining bytes on first dw */
	subfic	r9, r9, 8
	subf	r8, r9, r8		/* recalculate padding */
L(srcunaligndstalign):
	addi	rRTN, rRTN, 1
	subfic	r5, r10, 64		/* remaining bytes on second dw */
	addi	rSRC, rSRC, 8
	li	rTMP,0
	b	L(storedouble)

	.align 4
L(dst_align_small):
	mtctr	r8
	/* Write till src is aligned */
L(storebyte2):
#ifdef __LITTLE_ENDIAN__
	addi	r11, r11, 8		/* Adjust byte pointer on dw */
#else
	addi	r11, r11, -8
#endif
	srd	rALT, rWORD, r11
	stbu	rALT, 1(rRTN)
	bdnz	L(storebyte2)

	addi	rSRC, rSRC, 8		/* Increment src pointer */
	addi	rRTN, rRTN, 1		/* Increment dst pointer */
	rldicl	r8, rRTN, 0, 61		/* Recalculate padding */

	/* src is aligned */
L(srcaligndstunalign):
	ld	rWORD, 0(rSRC)
	mr	rALT, rWORD
	li	rTMP, 0			/* Check null */
	cmpb	rTMP, rWORD, rTMP
	cmpdi	rTMP, 0
	bne	L(bytebybyte)		/* Do byte by byte if there is NULL */
	rlwinm	r5, rRTN, 3,26,28	/* Calculate padding */
	addi	rRTN, rRTN, -1
	subfic	r10, r8, 8
	/* write byte by byte till aligned */
#ifdef __LITTLE_ENDIAN__
	li	r11, -8
#else
	li	r11, 64
#endif
	mtctr	r10
	cmpdi	rTMP, r10, 4
	blt	L(storebyte)
	beq	L(equal)
	addi	rTMP, r10, -4
	mtctr	rTMP
L(storebyte):
#ifdef __LITTLE_ENDIAN__
	addi	r11, r11, 8		/* Adjust byte pointer on  dw */
#else
	addi	r11, r11, -8
#endif
	srd	rALT, rWORD, r11
	stbu	rALT, 1(rRTN)
	bdnz	L(storebyte)

	cmpdi	rTMP, r10, 4
	blt	L(align)

	.align 4
L(equal):
#ifdef __LITTLE_ENDIAN__
	addi	r11, r11, 8
	srd	rALT, rWORD, r11
#else
	subfic	r11, r11, 64
	sld	rALT, rWORD, r11
	srdi	rALT, rALT, 32
#endif
	stw	rALT, 1(rRTN)
	addi	rRTN, rRTN, 4
L(align):
	addi	rRTN, rRTN, 1
	addi	rSRC, rSRC, 8		/* Increment src pointer */
	subfic	r10, r5, 64
	li	rTMP, 0
	/* dst addr aligned to 8 */
L(storedouble):
	ld	rALT, 0(rSRC)		/* load next dw */
	cmpb	rTMP, rALT, rTMP
	cmpdi	rTMP, 0			/* check for null on each new dw */
	bne	L(null)
#ifdef __LITTLE_ENDIAN__
	srd	r9, rWORD, r10		/* bytes from first dw */
	sld	r11, rALT, r5		/* bytes from second dw */
#else
	sld	r9, rWORD, r10
	srd	r11, rALT, r5
#endif
	or	r11, r9, r11		/* make as a single dw */
	std	r11, 0(rRTN)		/* store as std on aligned addr */
	mr	rWORD, rALT		/* still few bytes left to be written */
	addi	rRTN, rRTN, 8		/* increment dst addr */
	addi	rSRC, rSRC, 8		/* increment src addr */
	b	L(storedouble)		/* Loop till NULL */

	.align 4

/* We've hit the end of the string.  Do the rest byte-by-byte.  */
L(null):
	addi	rRTN, rRTN, -1
	mr	r10, r5
	mtctr	r8
#ifdef __LITTLE_ENDIAN__
	subfic	r10, r10, 64
	addi	r10, r10, -8
#endif
	cmpdi	rTMP, r8, 4
	blt	L(loop)

	/* we can still use stw if leftover >= 4*/
#ifdef __LITTLE_ENDIAN__
	addi	r10, r10, 8
	srd	r11, rWORD, r10
#else
	subfic	r10, r10, 64
	sld	r11, rWORD, r10
	srdi	r11, r11, 32
#endif
	stw	r11, 1(rRTN)
	addi	rRTN, rRTN, 4

	beq	L(bytebybyte1)
	addi	r10, r10, 32
#ifdef __LITTLE_ENDIAN__
	addi	r10, r10, -8
#else
	subfic	r10, r10, 64
#endif
	addi	rTMP, r8, -4
	mtctr	rTMP
	/* remaining byte by byte part of first dw */
L(loop):
#ifdef __LITTLE_ENDIAN__
	addi	r10, r10, 8
#else
	addi	r10, r10, -8
#endif
	srd	rTMP, rWORD, r10
	stbu	rTMP, 1(rRTN)
	bdnz	L(loop)

L(bytebybyte1):
	addi	rRTN, rRTN, 1
	/* remaining byte by byte part of second dw */
L(bytebybyte):
	addi	rRTN, rRTN, -8
	b	L(g1)

END (FUNC_NAME)

#ifndef USE_AS_STPCPY
libc_hidden_builtin_def (strcpy)
#endif
