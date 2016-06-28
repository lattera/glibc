/* Optimized sinf().  PowerPC64/POWER8 version.
   Copyright (C) 2016 Free Software Foundation, Inc.
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
#define _ERRNO_H	1
#include <bits/errno.h>

#define FRAMESIZE (FRAME_MIN_SIZE+16)

#define FLOAT_EXPONENT_SHIFT	23
#define FLOAT_EXPONENT_BIAS	127
#define INTEGER_BITS		3

#define PI_4		0x3f490fdb	/* PI/4 */
#define NINEPI_4	0x40e231d6	/* 9 * PI/4 */
#define TWO_PN5		0x3d000000	/* 2^-5 */
#define TWO_PN27	0x32000000	/* 2^-27 */
#define INFINITY	0x7f800000
#define TWO_P23		0x4b000000	/* 2^27 */
#define FX_FRACTION_1_28 0x9249250	/* 0x100000000 / 28 + 1 */

	/* Implements the function

	   float [fp1] sinf (float [fp1] x)  */

	.machine power8
EALIGN(__sinf, 4, 0)
	addis	r9,r2,L(anchor)@toc@ha
	addi	r9,r9,L(anchor)@toc@l

	lis	r4,PI_4@h
	ori	r4,r4,PI_4@l

	xscvdpspn v0,v1
	mfvsrd	r8,v0
	rldicl	r3,r8,32,33		/* Remove sign bit.  */

	cmpw	r3,r4
	bge	L(greater_or_equal_pio4)

	lis	r4,TWO_PN5@h
	ori	r4,r4,TWO_PN5@l

	cmpw	r3,r4
	blt	L(less_2pn5)

	/* Chebyshev polynomial of the form:
	 * x+x^3*(S0+x^2*(S1+x^2*(S2+x^2*(S3+x^2*S4)))).  */

	lfd	fp9,(L(S0)-L(anchor))(r9)
	lfd	fp10,(L(S1)-L(anchor))(r9)
	lfd	fp11,(L(S2)-L(anchor))(r9)
	lfd	fp12,(L(S3)-L(anchor))(r9)
	lfd	fp13,(L(S4)-L(anchor))(r9)

	fmul	fp2,fp1,fp1		/* x^2 */
	fmul	fp3,fp2,fp1		/* x^3 */

	fmadd	fp4,fp2,fp13,fp12	/* S3+x^2*S4 */
	fmadd	fp4,fp2,fp4,fp11	/* S2+x^2*(S3+x^2*S4) */
	fmadd	fp4,fp2,fp4,fp10	/* S1+x^2*(S2+x^2*(S3+x^2*S4)) */
	fmadd	fp4,fp2,fp4,fp9		/* S0+x^2*(S1+x^2*(S2+x^2*(S3+x^2*S4))) */
	fmadd	fp1,fp3,fp4,fp1		/* x+x^3*(S0+x^2*(S1+x^2*(S2+x^2*(S3+x^2*S4)))) */
	frsp	fp1,fp1			/* Round to single precision.  */

	blr

	.balign 16
L(greater_or_equal_pio4):
	lis	r4,NINEPI_4@h
	ori	r4,r4,NINEPI_4@l
	cmpw	r3,r4
	bge	L(greater_or_equal_9pio4)

	/* Calculate quotient of |x|/(PI/4).  */
	lfd	fp2,(L(invpio4)-L(anchor))(r9)
	fabs	fp1,fp1			/* |x| */
	fmul	fp2,fp1,fp2		/* |x|/(PI/4) */
	fctiduz	fp2,fp2
	mfvsrd	r3,v2			/* n = |x| mod PI/4 */

	/* Now use that quotient to find |x| mod (PI/2).  */
	addi	r7,r3,1
	rldicr	r5,r7,2,60		/* ((n+1) >> 1) << 3 */
	addi	r6,r9,(L(pio2_table)-L(anchor))
	lfdx	fp4,r5,r6
	fsub	fp1,fp1,fp4

	.balign 16
L(reduced):
	/* Now we are in the range -PI/4 to PI/4.  */

	/* Work out if we are in a positive or negative primary interval.  */
	rldicl	r4,r7,62,63		/* ((n+1) >> 2) & 1 */

	/* We are operating on |x|, so we need to add back the original
	   sign.  */
	rldicl	r8,r8,33,63		/* (x >> 31) & 1, ie the sign bit.  */
	xor	r4,r4,r8		/* 0 if result should be positive,
					   1 if negative.  */

	/* Load a 1.0 or -1.0.  */
	addi	r5,r9,(L(ones)-L(anchor))
	sldi	r4,r4,3
	lfdx	fp0,r4,r5

	/* Are we in the primary interval of sin or cos?  */
	andi.	r4,r7,0x2
	bne	L(cos)

	/* Chebyshev polynomial of the form:
	   x+x^3*(S0+x^2*(S1+x^2*(S2+x^2*(S3+x^2*S4)))).  */

	lfd	fp9,(L(S0)-L(anchor))(r9)
	lfd	fp10,(L(S1)-L(anchor))(r9)
	lfd	fp11,(L(S2)-L(anchor))(r9)
	lfd	fp12,(L(S3)-L(anchor))(r9)
	lfd	fp13,(L(S4)-L(anchor))(r9)

	fmul	fp2,fp1,fp1		/* x^2 */
	fmul	fp3,fp2,fp1		/* x^3 */

	fmadd	fp4,fp2,fp13,fp12	/* S3+x^2*S4 */
	fmadd	fp4,fp2,fp4,fp11	/* S2+x^2*(S3+x^2*S4) */
	fmadd	fp4,fp2,fp4,fp10	/* S1+x^2*(S2+x^2*(S3+x^2*S4)) */
	fmadd	fp4,fp2,fp4,fp9		/* S0+x^2*(S1+x^2*(S2+x^2*(S3+x^2*S4))) */
	fmadd	fp4,fp3,fp4,fp1		/* x+x^3*(S0+x^2*(S1+x^2*(S2+x^2*(S3+x^2*S4)))) */
	fmul	fp4,fp4,fp0		/* Add in the sign.  */
	frsp	fp1,fp4			/* Round to single precision.  */

	blr

	.balign 16
L(cos):
	/* Chebyshev polynomial of the form:
	   1.0+x^2*(C0+x^2*(C1+x^2*(C2+x^2*(C3+x^2*C4)))).  */

	lfd	fp9,(L(C0)-L(anchor))(r9)
	lfd	fp10,(L(C1)-L(anchor))(r9)
	lfd	fp11,(L(C2)-L(anchor))(r9)
	lfd	fp12,(L(C3)-L(anchor))(r9)
	lfd	fp13,(L(C4)-L(anchor))(r9)

	fmul	fp2,fp1,fp1		/* x^2 */
	lfd	fp3,(L(DPone)-L(anchor))(r9)

	fmadd	fp4,fp2,fp13,fp12	/* C3+x^2*C4 */
	fmadd	fp4,fp2,fp4,fp11	/* C2+x^2*(C3+x^2*C4) */
	fmadd	fp4,fp2,fp4,fp10	/* C1+x^2*(C2+x^2*(C3+x^2*C4)) */
	fmadd	fp4,fp2,fp4,fp9		/* C0+x^2*(C1+x^2*(C2+x^2*(C3+x^2*C4))) */
	fmadd	fp4,fp2,fp4,fp3		/* 1.0 + x^2*(C0+x^2*(C1+x^2*(C2+x^2*(C3+x^2*C4)))) */
	fmul	fp4,fp4,fp0		/* Add in the sign.  */
	frsp	fp1,fp4			/* Round to single precision.  */

	blr

	.balign 16
L(greater_or_equal_9pio4):
	lis	r4,INFINITY@h
	ori	r4,r4,INFINITY@l
	cmpw	r3,r4
	bge	L(inf_or_nan)

	lis	r4,TWO_P23@h
	ori	r4,r4,TWO_P23@l
	cmpw	r3,r4
	bge	L(greater_or_equal_2p23)

	fabs	fp1,fp1			/* |x| */

	/* Calculate quotient of |x|/(PI/4).  */
	lfd	fp2,(L(invpio4)-L(anchor))(r9)

	lfd	fp3,(L(DPone)-L(anchor))(r9)
	lfd	fp4,(L(DPhalf)-L(anchor))(r9)
	fmul	fp2,fp1,fp2		/* |x|/(PI/4) */
	friz	fp2,fp2			/* n = floor(|x|/(PI/4)) */

	/* Calculate (n + 1) / 2.  */
	fadd	fp2,fp2,fp3		/* n + 1 */
	fmul	fp3,fp2,fp4		/* (n + 1) / 2 */
	friz	fp3,fp3

	lfd	fp4,(L(pio2hi)-L(anchor))(r9)
	lfd	fp5,(L(pio2lo)-L(anchor))(r9)

	fmul	fp6,fp4,fp3
	fadd	fp6,fp6,fp1
	fmadd	fp1,fp5,fp3,fp6

	fctiduz	fp2,fp2
	mfvsrd	r7,v2			/* n + 1 */

	b	L(reduced)

	.balign 16
L(inf_or_nan):
	bne	L(skip_errno_setting)	/* Is a NAN?  */

	/* We delayed the creation of the stack frame, as well as the saving of
	   the link register, because only at this point, we are sure that
	   doing so is actually needed.  */

	stfd	fp1,-8(r1)

	/* Save the link register.  */
	mflr	r0
	std	r0,16(r1)
	cfi_offset(lr, 16)

	/* Create the stack frame.  */
	stdu	r1,-FRAMESIZE(r1)
	cfi_adjust_cfa_offset(FRAMESIZE)

	bl	JUMPTARGET(__errno_location)
	nop

	/* Restore the stack frame.  */
	addi	r1,r1,FRAMESIZE
	cfi_adjust_cfa_offset(-FRAMESIZE)
	/* Restore the link register.  */
	ld	r0,16(r1)
	mtlr	r0

	lfd	fp1,-8(r1)

	/* errno = EDOM */
	li	r4,EDOM
	stw	r4,0(r3)

L(skip_errno_setting):
	fsub	fp1,fp1,fp1		/* x - x */
	blr

	.balign 16
L(greater_or_equal_2p23):
	fabs	fp1,fp1

	srwi	r4,r3,FLOAT_EXPONENT_SHIFT
	subi	r4,r4,FLOAT_EXPONENT_BIAS

	/* We reduce the input modulo pi/4, so we need 3 bits of integer
	   to determine where in 2*pi we are. Index into our array
	   accordingly.  */
	addi r4,r4,INTEGER_BITS

	/* To avoid an expensive divide, for the range we care about (0 - 127)
	   we can transform x/28 into:

	   x/28 = (x * ((0x100000000 / 28) + 1)) >> 32

	   mulhwu returns the top 32 bits of the 64 bit result, doing the
	   shift for us in the same instruction. The top 32 bits are undefined,
	   so we have to mask them.  */

	lis	r6,FX_FRACTION_1_28@h
	ori	r6,r6,FX_FRACTION_1_28@l
	mulhwu	r5,r4,r6
	clrldi	r5,r5,32

	/* Get our pointer into the invpio4_table array.  */
	sldi	r4,r5,3
	addi	r6,r9,(L(invpio4_table)-L(anchor))
	add	r4,r4,r6

	lfd	fp2,0(r4)
	lfd	fp3,8(r4)
	lfd	fp4,16(r4)
	lfd	fp5,24(r4)

	fmul	fp6,fp2,fp1
	fmul	fp7,fp3,fp1
	fmul	fp8,fp4,fp1
	fmul	fp9,fp5,fp1

	/* Mask off larger integer bits in highest double word that we don't
	   care about to avoid losing precision when combining with smaller
	   values.  */
	fctiduz	fp10,fp6
	mfvsrd	r7,v10
	rldicr	r7,r7,0,(63-INTEGER_BITS)
	mtvsrd	v10,r7
	fcfidu	fp10,fp10		/* Integer bits.  */

	fsub	fp6,fp6,fp10		/* highest -= integer bits */

	/* Work out the integer component, rounded down. Use the top two
	   limbs for this.  */
	fadd	fp10,fp6,fp7		/* highest + higher */

	fctiduz	fp10,fp10
	mfvsrd	r7,v10
	andi.	r0,r7,1
	fcfidu	fp10,fp10

	/* Subtract integer component from highest limb.  */
	fsub	fp12,fp6,fp10

	beq	L(even_integer)

	/* Our integer component is odd, so we are in the -PI/4 to 0 primary
	   region. We need to shift our result down by PI/4, and to do this
	   in the mod (4/PI) space we simply subtract 1.  */
	lfd	fp11,(L(DPone)-L(anchor))(r9)
	fsub	fp12,fp12,fp11

	/* Now add up all the limbs in order.  */
	fadd	fp12,fp12,fp7
	fadd	fp12,fp12,fp8
	fadd	fp12,fp12,fp9

	/* And finally multiply by pi/4.  */
	lfd	fp13,(L(pio4)-L(anchor))(r9)
	fmul	fp1,fp12,fp13

	addi	r7,r7,1
	b	L(reduced)

L(even_integer):
	lfd	fp11,(L(DPone)-L(anchor))(r9)

	/* Now add up all the limbs in order.  */
	fadd	fp12,fp12,fp7
	fadd	fp12,r12,fp8
	fadd	fp12,r12,fp9

	/* We need to check if the addition of all the limbs resulted in us
	   overflowing 1.0.  */
	fcmpu	0,fp12,fp11
	bgt	L(greater_than_one)

	/* And finally multiply by pi/4.  */
	lfd	fp13,(L(pio4)-L(anchor))(r9)
	fmul	fp1,fp12,fp13

	addi	r7,r7,1
	b	L(reduced)

L(greater_than_one):
	/* We did overflow 1.0 when adding up all the limbs. Add 1.0 to our
	   integer, and subtract 1.0 from our result. Since that makes the
	   integer component odd, we need to subtract another 1.0 as
	   explained above.  */
	addi	r7,r7,1

	lfd	fp11,(L(DPtwo)-L(anchor))(r9)
	fsub	fp12,fp12,fp11

	/* And finally multiply by pi/4.  */
	lfd	fp13,(L(pio4)-L(anchor))(r9)
	fmul	fp1,fp12,fp13

	addi	r7,r7,1
	b	L(reduced)

	.balign 16
L(less_2pn5):
	lis	r4,TWO_PN27@h
	ori	r4,r4,TWO_PN27@l

	cmpw	r3,r4
	blt	L(less_2pn27)

	/* A simpler Chebyshev approximation is close enough for this range:
	   x+x^3*(SS0+x^2*SS1).  */

	lfd	fp10,(L(SS0)-L(anchor))(r9)
	lfd	fp11,(L(SS1)-L(anchor))(r9)

	fmul	fp2,fp1,fp1		/* x^2 */
	fmul	fp3,fp2,fp1		/* x^3 */

	fmadd	fp4,fp2,fp11,fp10	/* SS0+x^2*SS1 */
	fmadd	fp1,fp3,fp4,fp1		/* x+x^3*(SS0+x^2*SS1) */

	frsp	fp1,fp1			/* Round to single precision.  */

	blr

	.balign 16
L(less_2pn27):
	cmpwi	r3,0
	beq	L(zero)

	/* Handle some special cases:

	   sinf(subnormal) raises inexact/underflow
	   sinf(min_normalized) raises inexact/underflow
	   sinf(normalized) raises inexact.  */

	lfd	fp2,(L(small)-L(anchor))(r9)

	fmul	fp2,fp1,fp2		/* x * small */
	fsub	fp1,fp1,fp2		/* x - x * small */

	frsp	fp1,fp1

	blr

	.balign 16
L(zero):
	blr

END (__sinf)

	.section .rodata, "a"

	.balign 8

L(anchor):

	/* Chebyshev constants for sin, range -PI/4 - PI/4.  */
L(S0):	.8byte	0xbfc5555555551cd9
L(S1):	.8byte	0x3f81111110c2688b
L(S2):	.8byte	0xbf2a019f8b4bd1f9
L(S3):	.8byte	0x3ec71d7264e6b5b4
L(S4):	.8byte	0xbe5a947e1674b58a

	/* Chebyshev constants for sin, range 2^-27 - 2^-5.  */
L(SS0):	.8byte	0xbfc555555543d49d
L(SS1):	.8byte	0x3f8110f475cec8c5

	/* Chebyshev constants for cos, range -PI/4 - PI/4.  */
L(C0):	.8byte	0xbfdffffffffe98ae
L(C1):	.8byte	0x3fa55555545c50c7
L(C2):	.8byte	0xbf56c16b348b6874
L(C3):	.8byte	0x3efa00eb9ac43cc0
L(C4):	.8byte	0xbe923c97dd8844d7

L(invpio2):
	.8byte	0x3fe45f306dc9c883	/* 2/PI */

L(invpio4):
	.8byte	0x3ff45f306dc9c883	/* 4/PI */

L(invpio4_table):
	.8byte	0x0000000000000000
	.8byte	0x3ff45f306c000000
	.8byte	0x3e3c9c882a000000
	.8byte	0x3c54fe13a8000000
	.8byte	0x3aaf47d4d0000000
	.8byte	0x38fbb81b6c000000
	.8byte	0x3714acc9e0000000
	.8byte	0x3560e4107c000000
	.8byte	0x33bca2c756000000
	.8byte	0x31fbd778ac000000
	.8byte	0x300b7246e0000000
	.8byte	0x2e5d2126e8000000
	.8byte	0x2c97003248000000
	.8byte	0x2ad77504e8000000
	.8byte	0x290921cfe0000000
	.8byte	0x274deb1cb0000000
	.8byte	0x25829a73e0000000
	.8byte	0x23fd1046be000000
	.8byte	0x2224baed10000000
	.8byte	0x20709d338e000000
	.8byte	0x1e535a2f80000000
	.8byte	0x1cef904e64000000
	.8byte	0x1b0d639830000000
	.8byte	0x1964ce7d24000000
	.8byte	0x17b908bf16000000

L(pio4):
	.8byte	0x3fe921fb54442d18	/* PI/4 */

/* PI/2 as a sum of two doubles. We only use 32 bits of the upper limb
   to avoid losing significant bits when multiplying with up to
   (2^22)/(pi/2).  */
L(pio2hi):
	.8byte	0xbff921fb54400000

L(pio2lo):
	.8byte	0xbdd0b4611a626332

L(pio2_table):
	.8byte	0
	.8byte	0x3ff921fb54442d18	/* 1 * PI/2 */
	.8byte	0x400921fb54442d18	/* 2 * PI/2 */
	.8byte	0x4012d97c7f3321d2	/* 3 * PI/2 */
	.8byte	0x401921fb54442d18	/* 4 * PI/2 */
	.8byte	0x401f6a7a2955385e	/* 5 * PI/2 */
	.8byte	0x4022d97c7f3321d2	/* 6 * PI/2 */
	.8byte	0x4025fdbbe9bba775	/* 7 * PI/2 */
	.8byte	0x402921fb54442d18	/* 8 * PI/2 */
	.8byte	0x402c463abeccb2bb	/* 9 * PI/2 */
	.8byte	0x402f6a7a2955385e	/* 10 * PI/2 */

L(small):
	.8byte	0x3cd0000000000000	/* 2^-50 */

L(ones):
	.8byte	0x3ff0000000000000	/* +1.0 */
	.8byte	0xbff0000000000000	/* -1.0 */

L(DPhalf):
	.8byte	0x3fe0000000000000	/* 0.5 */

L(DPone):
	.8byte	0x3ff0000000000000	/* 1.0 */

L(DPtwo):
	.8byte	0x4000000000000000	/* 2.0 */

weak_alias(__sinf, sinf)
