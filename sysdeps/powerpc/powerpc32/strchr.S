/* Optimized strchr implementation for PowerPC.
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

/* See strlen.s for comments on how this works.  */

/* char * [r3] strchr (const char *s [r3] , int c [r4] )  */

ENTRY (BP_SYM (strchr))

#define rTMP1	r0
#define rRTN	r3	/* outgoing result */
#if __BOUNDED_POINTERS__
# define rSTR	r4
# define rCHR	r5	/* byte we're looking for, spread over the whole word */
# define rWORD	r8	/* the current word */
#else
# define rSTR	r8	/* current word pointer */
# define rCHR	r4	/* byte we're looking for, spread over the whole word */
# define rWORD	r5	/* the current word */
#endif
#define rCLZB	rCHR	/* leading zero byte count */
#define rFEFE	r6	/* constant 0xfefefeff (-0x01010101) */
#define r7F7F	r7	/* constant 0x7f7f7f7f */
#define rTMP2	r9
#define rIGN	r10	/* number of bits we should ignore in the first word */
#define rMASK	r11	/* mask with the bits to ignore set to 0 */
#define rTMP3	r12

	CHECK_BOUNDS_LOW (rSTR, rTMP1, rTMP2)
	STORE_RETURN_BOUNDS (rTMP1, rTMP2)

	rlwimi	rCHR, rCHR, 8, 16, 23
	li	rMASK, -1
	rlwimi	rCHR, rCHR, 16, 0, 15
	rlwinm	rIGN, rRTN, 3, 27, 28
	lis	rFEFE, -0x101
	lis	r7F7F, 0x7f7f
	clrrwi	rSTR, rRTN, 2
	addi	rFEFE, rFEFE, -0x101
	addi	r7F7F, r7F7F, 0x7f7f
/* Test the first (partial?) word.  */
	lwz	rWORD, 0(rSTR)
	srw	rMASK, rMASK, rIGN
	orc	rWORD, rWORD, rMASK
	add	rTMP1, rFEFE, rWORD
	nor	rTMP2, r7F7F, rWORD
	and.	rTMP1, rTMP1, rTMP2
	xor	rTMP3, rCHR, rWORD
	orc	rTMP3, rTMP3, rMASK
	b	L(loopentry)

/* The loop.  */

L(loop):lwzu rWORD, 4(rSTR)
	and.	rTMP1, rTMP1, rTMP2
/* Test for 0.	*/
	add	rTMP1, rFEFE, rWORD
	nor	rTMP2, r7F7F, rWORD
	bne	L(foundit)
	and.	rTMP1, rTMP1, rTMP2
/* Start test for the bytes we're looking for.  */
	xor	rTMP3, rCHR, rWORD
L(loopentry):
	add	rTMP1, rFEFE, rTMP3
	nor	rTMP2, r7F7F, rTMP3
	beq	L(loop)
/* There is a zero byte in the word, but may also be a matching byte (either
   before or after the zero byte).  In fact, we may be looking for a
   zero byte, in which case we return a match.  We guess that this hasn't
   happened, though.  */
L(missed):
	and.	rTMP1, rTMP1, rTMP2
	li	rRTN, 0
	STORE_RETURN_VALUE (rSTR)
	beqlr
/* It did happen. Decide which one was first...
   I'm not sure if this is actually faster than a sequence of
   rotates, compares, and branches (we use it anyway because it's shorter).  */
	and	rFEFE, r7F7F, rWORD
	or	rMASK, r7F7F, rWORD
	and	rTMP1, r7F7F, rTMP3
	or	rIGN, r7F7F, rTMP3
	add	rFEFE, rFEFE, r7F7F
	add	rTMP1, rTMP1, r7F7F
	nor	rWORD, rMASK, rFEFE
	nor	rTMP2, rIGN, rTMP1
	cmplw	rWORD, rTMP2
	bgtlr
	cntlzw	rCLZB, rTMP2
	srwi	rCLZB, rCLZB, 3
	add	rRTN, rSTR, rCLZB
	CHECK_BOUNDS_HIGH_RTN (rSTR, rTMP2, twlge)
	STORE_RETURN_VALUE (rSTR)
	blr

L(foundit):
	and	rTMP1, r7F7F, rTMP3
	or	rIGN, r7F7F, rTMP3
	add	rTMP1, rTMP1, r7F7F
	nor	rTMP2, rIGN, rTMP1
	cntlzw	rCLZB, rTMP2
	subi	rSTR, rSTR, 4
	srwi	rCLZB, rCLZB, 3
	add	rRTN, rSTR, rCLZB
	CHECK_BOUNDS_HIGH_RTN (rSTR, rTMP2, twlge)
	STORE_RETURN_VALUE (rSTR)
	blr
END (BP_SYM (strchr))

weak_alias (BP_SYM (strchr), BP_SYM (index))
libc_hidden_builtin_def (strchr)
