/* Optimized strcpy implementation for PowerPC64.
   Copyright (C) 1997, 1999, 2000, 2002, 2003 Free Software Foundation, Inc.
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

/* char * [r3] strcpy (char *dest [r3], const char *src [r4])  */

EALIGN (BP_SYM (strcpy), 4, 0)
	CALL_MCOUNT 2

#define rTMP	r0
#define rRTN	r3	/* incoming DEST arg preserved as result */
/* Note.  The Bounded pointer support in this code is broken.  This code
   was inherited from PPC32 and and that support was never completed.  
   Current PPC gcc does not support -fbounds-check or -fbounded-pointers.
   These artifacts are left in the code as a reminder in case we need
   bounded pointer support in the future.  */
#if __BOUNDED_POINTERS__
# define rDEST	r4	/* pointer to previous word in dest */
# define rSRC	r5	/* pointer to previous word in src */
# define rLOW	r11
# define rHIGH	r12
#else
# define rSRC	r4	/* pointer to previous word in src */
# define rDEST	r5	/* pointer to previous word in dest */
#endif
#define rWORD	r6	/* current word from src */
#define rFEFE	r7	/* constant 0xfefefefefefefeff (-0x0101010101010101) */
#define r7F7F	r8	/* constant 0x7f7f7f7f7f7f7f7f */
#define rNEG	r9	/* ~(word in s1 | 0x7f7f7f7f7f7f7f7f) */
#define rALT	r10	/* alternate word from src */

	CHECK_BOUNDS_LOW (rSRC, rLOW, rHIGH)
	CHECK_BOUNDS_LOW (rDEST, rLOW, rHIGH)
	STORE_RETURN_BOUNDS (rLOW, rHIGH)

	dcbt	0,rSRC
	or	rTMP, rSRC, rRTN
	clrldi.	rTMP, rTMP, 61
#if __BOUNDED_POINTERS__
	addi	rDEST, rDEST, -8
#else
	addi	rDEST, rRTN, -8
#endif
	dcbtst	0,rRTN
	bne	L(unaligned)

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
	stdu	rWORD, 8(rDEST)
	add	rTMP, rFEFE, rALT
	nor	rNEG, r7F7F, rALT
	and.	rTMP, rTMP, rNEG
	bne-	L(g1)
	ldu	rWORD, 8(rSRC)
	stdu	rALT, 8(rDEST)
L(g2):	add	rTMP, rFEFE, rWORD
	nor	rNEG, r7F7F, rWORD
	and.	rTMP, rTMP, rNEG
	beq+	L(g0)

	mr	rALT, rWORD
/* We've hit the end of the string.  Do the rest byte-by-byte.  */
L(g1):
	extrdi.	rTMP, rALT, 8, 0
	stb	rTMP, 8(rDEST)
	beqlr-
	extrdi.	rTMP, rALT, 8, 8
	stb	rTMP, 9(rDEST)
	beqlr-
	extrdi.	rTMP, rALT, 8, 16
	stb	rTMP, 10(rDEST)
	beqlr-
	extrdi.	rTMP, rALT, 8, 24
	stb	rTMP, 11(rDEST)
	beqlr-
	extrdi.	rTMP, rALT, 8, 32
	stb	rTMP, 12(rDEST)
	beqlr-
	extrdi.	rTMP, rALT, 8, 40
	stb	rTMP, 13(rDEST)
	beqlr-
	extrdi.	rTMP, rALT, 8, 48
	stb	rTMP, 14(rDEST)
	beqlr-
	stb	rALT, 15(rDEST)
	/* GKM FIXME: check high bound.  */
	blr

/* Oh well.  In this case, we just do a byte-by-byte copy.  */
	.align 4
	nop
L(unaligned):
	lbz	rWORD, 0(rSRC)
	addi	rDEST, rRTN, -1
	cmpwi	rWORD, 0
	beq-	L(u2)

L(u0):	lbzu	rALT, 1(rSRC)
	stbu	rWORD, 1(rDEST)
	cmpwi	rALT, 0
	beq-	L(u1)
	nop		/* Let 601 load start of loop.  */
	lbzu	rWORD, 1(rSRC)
	stbu	rALT, 1(rDEST)
	cmpwi	rWORD, 0
	bne+	L(u0)
L(u2):	stb	rWORD, 1(rDEST)
	/* GKM FIXME: check high bound.  */
	blr
L(u1):	stb	rALT, 1(rDEST)
	/* GKM FIXME: check high bound.  */
	blr

END (BP_SYM (strcpy))
libc_hidden_builtin_def (strcpy)
