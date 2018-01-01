/* Optimized expf().  PowerPC64/POWER8 version.
   Copyright (C) 2016-2018 Free Software Foundation, Inc.
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

/* Short algorithm description:
 *
 *  Let K = 64 (table size).
 *       e^x  = 2^(x/log(2)) = 2^n * T[j] * (1 + P(y))
 *  where:
 *       x = m*log(2)/K + y,    y in [0.0..log(2)/K]
 *       m = n*K + j,           m,n,j - signed integer, j in [0..K-1]
 *       values of 2^(j/K) are tabulated as T[j].
 *
 *       P(y) is a minimax polynomial approximation of expf(y)-1
 *       on small interval [0.0..log(2)/K].
 *
 *       P(y) = P3*y*y*y*y + P2*y*y*y + P1*y*y + P0*y, calculated as
 *       z = y*y;    P(y) = (P3*z + P1)*z + (P2*z + P0)*y
 *
 * Special cases:
 *  expf(NaN) = NaN
 *  expf(+INF) = +INF
 *  expf(-INF) = 0
 *  expf(x) = 1 for subnormals
 *  for finite argument, only expf(0)=1 is exact
 *  expf(x) overflows if x>88.7228317260742190
 *  expf(x) underflows if x<-103.972076416015620
 */

#define C1 0x42ad496b		/* Single precision 125*log(2).  */
#define C2 0x31800000		/* Single precision 2^(-28).  */
#define SP_INF 0x7f800000	/* Single precision Inf.  */
#define SP_EXP_BIAS 0x1fc0	/* Single precision exponent bias.  */

#define DATA_OFFSET r9

/* Implements the function

   float [fp1] expf (float [fp1] x)  */

	.machine power8
ENTRY (__ieee754_expf, 4)
	addis	DATA_OFFSET,r2,.Lanchor@toc@ha
	addi	DATA_OFFSET,DATA_OFFSET,.Lanchor@toc@l

	xscvdpspn v0,v1
	mfvsrd	r8,v0		/* r8 = x  */
	lfd	fp2,(.KLN2-.Lanchor)(DATA_OFFSET)
	lfd	fp3,(.P2-.Lanchor)(DATA_OFFSET)
	rldicl	r3,r8,32,33	/* r3 = |x|  */
	lis	r4,C1@ha	/* r4 = 125*log(2)  */
	ori	r4,r4,C1@l
	cmpw	r3,r4
	lfd	fp5,(.P3-.Lanchor)(DATA_OFFSET)
	lfd	fp4,(.RS-.Lanchor)(DATA_OFFSET)
	fmadd	fp2,fp1,fp2,fp4	/* fp2 = x * K/log(2) + (2^23 + 2^22)  */
	bge	L(special_paths)	/* |x| >= 125*log(2) ?  */

	lis	r4,C2@ha
	ori	r4,r4,C2@l
	cmpw	r3,r4
	blt	L(small_args)	/* |x| < 2^(-28) ?  */

	/* Main path: here if 2^(-28) <= |x| < 125*log(2) */
	frsp	fp6,fp2
	xscvdpsp v2,v2
	mfvsrd	r8,v2
	mr	r3,r8			/* r3 = m  */
	rldicl	r8,r8,32,58		/* r8 = j  */
	lfs	fp4,(.SP_RS-.Lanchor)(DATA_OFFSET)
	fsubs	fp2,fp6,fp4		/* fp2 = m = x * K/log(2)  */
	srdi	r3,r3,32
	clrrwi	r3,r3,6			/* r3 = n  */
	lfd	fp6,(.NLN2K-.Lanchor)(DATA_OFFSET)
	fmadd	fp0,fp2,fp6,fp1		/* fp0 = y = x - m*log(2)/K  */
	fmul	fp2,fp0,fp0		/* fp2 = z = y^2  */
	lfd	fp4,(.P1-.Lanchor)(DATA_OFFSET)
	lfd	fp6,(.P0-.Lanchor)(DATA_OFFSET)
	lis	r4,SP_EXP_BIAS@ha
	ori	r4,r4,SP_EXP_BIAS@l
	add	r3,r3,r4
	rldic	r3,r3,49,1		/* r3 = 2^n  */
	fmadd	fp4,fp5,fp2,fp4		/* fp4 = P3 * z + P1  */
	fmadd	fp6,fp3,fp2,fp6		/* fp6 = P2 * z + P0  */
	mtvsrd	v1,r3
	xscvspdp v1,v1
	fmul	fp4,fp4,fp2		/* fp4 = (P3 * z + P1)*z  */
	fmadd	fp0,fp0,fp6,fp4		/* fp0 = P(y)  */
	sldi	r8,r8,3			/* Access doublewords from T[j].  */
	addi	r6,DATA_OFFSET,(.Ttable-.Lanchor)
	lfdx	fp3,r6,r8
	fmadd	fp0,fp0,fp3,fp3		/* fp0 = T[j] * (1 + P(y))  */
	fmul	fp1,fp1,fp0		/* fp1 = 2^n * T[j] * (1 + P(y))  */
	frsp	fp1,fp1
	blr

	.align	4
/* x is either underflow, overflow, infinite or NaN.  */
L(special_paths):
	srdi	r8,r8,32
	rlwinm	r8,r8,3,29,29		/* r8 = 0, if x positive.
					   r8 = 4, otherwise.  */
	addi	r6,DATA_OFFSET,(.SPRANGE-.Lanchor)
	lwzx	r4,r6,r8		/* r4 = .SPRANGE[signbit(x)]  */
	cmpw	r3,r4
	/* |x| <= .SPRANGE[signbit(x)]  */
	ble	L(near_under_or_overflow)

	lis	r4,SP_INF@ha
	ori	r4,r4,SP_INF@l
	cmpw	r3,r4
	bge	L(arg_inf_or_nan)	/* |x| > Infinite ?  */

	addi	r6,DATA_OFFSET,(.SPLARGE_SMALL-.Lanchor)
	lfsx	fp1,r6,r8
	fmuls	fp1,fp1,fp1
	blr


	.align	4
L(small_args):
	/* expf(x) = 1.0, where |x| < |2^(-28)|  */
	lfs	fp2,(.SPone-.Lanchor)(DATA_OFFSET)
	fadds	fp1,fp1,fp2
	blr


	.align	4
L(arg_inf_or_nan:)
	bne	L(arg_nan)

	/* expf(+INF) = +INF
	   expf(-INF) = 0  */
	addi	r6,DATA_OFFSET,(.INF_ZERO-.Lanchor)
	lfsx	fp1,r6,r8
	blr


	.align	4
L(arg_nan):
	/* expf(NaN) = NaN  */
	fadd	fp1,fp1,fp1
	frsp	fp1,fp1
	blr

	.align	4
L(near_under_or_overflow):
	frsp	fp6,fp2
	xscvdpsp v2,v2
	mfvsrd	r8,v2
	mr	r3,r8			/* r3 = m  */
	rldicl	r8,r8,32,58		/* r8 = j  */
	lfs	fp4,(.SP_RS-.Lanchor)(DATA_OFFSET)
	fsubs	fp2,fp6,fp4		/* fp2 = m = x * K/log(2)  */
	srdi	r3,r3,32
	clrrwi	r3,r3,6			/* r3 = n  */
	lfd	fp6,(.NLN2K-.Lanchor)(DATA_OFFSET)
	fmadd	fp0,fp2,fp6,fp1		/* fp0 = y = x - m*log(2)/K  */
	fmul	fp2,fp0,fp0		/* fp2 = z = y^2  */
	lfd	fp4,(.P1-.Lanchor)(DATA_OFFSET)
	lfd	fp6,(.P0-.Lanchor)(DATA_OFFSET)
	ld	r4,(.DP_EXP_BIAS-.Lanchor)(DATA_OFFSET)
	add	r3,r3,r4
	rldic	r3,r3,46,1		/* r3 = 2  */
	fmadd	fp4,fp5,fp2,fp4		/* fp4 = P3 * z + P1  */
	fmadd	fp6,fp3,fp2,fp6		/* fp6 = P2 * z + P0  */
	mtvsrd	v1,r3
	fmul	fp4,fp4,fp2		/* fp4 = (P3*z + P1)*z  */
	fmadd	fp0,fp0,fp6,fp4		/* fp0 = P(y)  */
	sldi	r8,r8,3			/* Access doublewords from T[j].  */
	addi	r6,DATA_OFFSET,(.Ttable-.Lanchor)
	lfdx	fp3,r6,r8
	fmadd	fp0,fp0,fp3,fp3		/* fp0 = T[j] * (1 + T[j])  */
	fmul	fp1,fp1,fp0		/* fp1 = 2^n * T[j] * (1 + T[j])  */
	frsp	fp1,fp1
	blr
END(__ieee754_expf)

	.section .rodata, "a",@progbits
.Lanchor:
	.balign	8
/* Table T[j] = 2^(j/K).  Double precision.  */
.Ttable:
	.8byte	0x3ff0000000000000
	.8byte	0x3ff02c9a3e778061
	.8byte	0x3ff059b0d3158574
	.8byte	0x3ff0874518759bc8
	.8byte	0x3ff0b5586cf9890f
	.8byte	0x3ff0e3ec32d3d1a2
	.8byte	0x3ff11301d0125b51
	.8byte	0x3ff1429aaea92de0
	.8byte	0x3ff172b83c7d517b
	.8byte	0x3ff1a35beb6fcb75
	.8byte	0x3ff1d4873168b9aa
	.8byte	0x3ff2063b88628cd6
	.8byte	0x3ff2387a6e756238
	.8byte	0x3ff26b4565e27cdd
	.8byte	0x3ff29e9df51fdee1
	.8byte	0x3ff2d285a6e4030b
	.8byte	0x3ff306fe0a31b715
	.8byte	0x3ff33c08b26416ff
	.8byte	0x3ff371a7373aa9cb
	.8byte	0x3ff3a7db34e59ff7
	.8byte	0x3ff3dea64c123422
	.8byte	0x3ff4160a21f72e2a
	.8byte	0x3ff44e086061892d
	.8byte	0x3ff486a2b5c13cd0
	.8byte	0x3ff4bfdad5362a27
	.8byte	0x3ff4f9b2769d2ca7
	.8byte	0x3ff5342b569d4f82
	.8byte	0x3ff56f4736b527da
	.8byte	0x3ff5ab07dd485429
	.8byte	0x3ff5e76f15ad2148
	.8byte	0x3ff6247eb03a5585
	.8byte	0x3ff6623882552225
	.8byte	0x3ff6a09e667f3bcd
	.8byte	0x3ff6dfb23c651a2f
	.8byte	0x3ff71f75e8ec5f74
	.8byte	0x3ff75feb564267c9
	.8byte	0x3ff7a11473eb0187
	.8byte	0x3ff7e2f336cf4e62
	.8byte	0x3ff82589994cce13
	.8byte	0x3ff868d99b4492ed
	.8byte	0x3ff8ace5422aa0db
	.8byte	0x3ff8f1ae99157736
	.8byte	0x3ff93737b0cdc5e5
	.8byte	0x3ff97d829fde4e50
	.8byte	0x3ff9c49182a3f090
	.8byte	0x3ffa0c667b5de565
	.8byte	0x3ffa5503b23e255d
	.8byte	0x3ffa9e6b5579fdbf
	.8byte	0x3ffae89f995ad3ad
	.8byte	0x3ffb33a2b84f15fb
	.8byte	0x3ffb7f76f2fb5e47
	.8byte	0x3ffbcc1e904bc1d2
	.8byte	0x3ffc199bdd85529c
	.8byte	0x3ffc67f12e57d14b
	.8byte	0x3ffcb720dcef9069
	.8byte	0x3ffd072d4a07897c
	.8byte	0x3ffd5818dcfba487
	.8byte	0x3ffda9e603db3285
	.8byte	0x3ffdfc97337b9b5f
	.8byte	0x3ffe502ee78b3ff6
	.8byte	0x3ffea4afa2a490da
	.8byte	0x3ffefa1bee615a27
	.8byte	0x3fff50765b6e4540
	.8byte	0x3fffa7c1819e90d8

.KLN2:
	.8byte	0x40571547652b82fe	/* Double precision K/log(2).  */

/* Double precision polynomial coefficients.  */
.P0:
	.8byte	0x3fefffffffffe7c6
.P1:
	.8byte	0x3fe00000008d6118
.P2:
	.8byte	0x3fc55550da752d4f
.P3:
	.8byte	0x3fa56420eb78fa85

.RS:
	.8byte	0x4168000000000000	/* Double precision 2^23 + 2^22.  */
.NLN2K:
	.8byte	0xbf862e42fefa39ef	/* Double precision -log(2)/K.  */
.DP_EXP_BIAS:
	.8byte	0x000000000000ffc0	/* Double precision exponent bias.  */

	.balign	4
.SPone:
	.4byte	0x3f800000	/* Single precision 1.0.  */
.SP_RS:
	.4byte	0x4b400000	/* Single precision 2^23 + 2^22.  */

.SPRANGE: /* Single precision overflow/underflow bounds.  */
	.4byte	0x42b17217	/* if x>this bound, then result overflows.  */
	.4byte	0x42cff1b4	/* if x<this bound, then result underflows.  */

.SPLARGE_SMALL:
	.4byte	0x71800000	/* 2^100.  */
	.4byte	0x0d800000	/* 2^-100.  */

.INF_ZERO:
	.4byte	0x7f800000	/* Single precision Inf.  */
	.4byte	0		/* Single precision zero.  */

strong_alias (__ieee754_expf, __expf_finite)
