/* Optimized stpcpy implementation for PowerPC.
   Copyright (C) 1997, 1999, 2000, 2002, 2004 Free Software Foundation, Inc.
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

/* char * [r3] stpcpy (char *dest [r3], const char *src [r4])  */

EALIGN (BP_SYM (__stpcpy), 4, 0)

#define rTMP	r0
#define rRTN	r3
#if __BOUNDED_POINTERS__
# define rDEST	r4		/* pointer to previous word in dest */
# define rSRC	r5		/* pointer to previous word in src */
# define rLOW	r11
# define rHIGH	r12
#else
# define rDEST	r3		/* pointer to previous word in dest */
# define rSRC	r4		/* pointer to previous word in src */
#endif
#define rWORD	r6		/* current word from src */
#define rFEFE	r7		/* 0xfefefeff */
#define r7F7F	r8		/* 0x7f7f7f7f */
#define rNEG	r9		/* ~(word in src | 0x7f7f7f7f) */
#define rALT	r10		/* alternate word from src */

	CHECK_BOUNDS_LOW (rSRC, rLOW, rHIGH)
	CHECK_BOUNDS_LOW (rDEST, rLOW, rHIGH)
	STORE_RETURN_BOUNDS (rLOW, rHIGH)

	or	rTMP, rSRC, rDEST
	clrlwi.	rTMP, rTMP, 30
	addi	rDEST, rDEST, -4
	bne	L(unaligned)

	lis	rFEFE, -0x101
	lis	r7F7F, 0x7f7f
	lwz	rWORD, 0(rSRC)
	addi	rFEFE, rFEFE, -0x101
	addi	r7F7F, r7F7F, 0x7f7f
	b	L(g2)

L(g0):	lwzu	rALT, 4(rSRC)
	stwu	rWORD, 4(rDEST)
	add	rTMP, rFEFE, rALT
	nor	rNEG, r7F7F, rALT
	and.	rTMP, rTMP, rNEG
	bne-	L(g1)
	lwzu	rWORD, 4(rSRC)
	stwu	rALT, 4(rDEST)
L(g2):	add	rTMP, rFEFE, rWORD
	nor	rNEG, r7F7F, rWORD
	and.	rTMP, rTMP, rNEG
	beq+	L(g0)

	mr	rALT, rWORD
/* We've hit the end of the string.  Do the rest byte-by-byte.  */
L(g1):	rlwinm.	rTMP, rALT, 8, 24, 31
	stbu	rTMP, 4(rDEST)
	beqlr-
	rlwinm.	rTMP, rALT, 16, 24, 31
	stbu	rTMP, 1(rDEST)
	beqlr-
	rlwinm.	rTMP, rALT, 24, 24, 31
	stbu	rTMP, 1(rDEST)
	beqlr-
	stbu	rALT, 1(rDEST)
	CHECK_BOUNDS_HIGH (rDEST, rHIGH, twlgt)
	STORE_RETURN_VALUE (rDEST)
	blr

/* Oh well.  In this case, we just do a byte-by-byte copy.  */
	.align 4
	nop
L(unaligned):
	lbz	rWORD, 0(rSRC)
	addi	rDEST, rDEST, 3
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
L(u2):	stbu	rWORD, 1(rDEST)
	CHECK_BOUNDS_HIGH (rDEST, rHIGH, twlgt)
	STORE_RETURN_VALUE (rDEST)
	blr
L(u1):	stbu	rALT, 1(rDEST)
	CHECK_BOUNDS_HIGH (rDEST, rHIGH, twlgt)
	STORE_RETURN_VALUE (rDEST)
	blr
END (BP_SYM (__stpcpy))

weak_alias (BP_SYM (__stpcpy), BP_SYM (stpcpy))
libc_hidden_def (__stpcpy)
libc_hidden_builtin_def (stpcpy)
