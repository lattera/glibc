/* lround function.  PowerPC32 version.
   Copyright (C) 2004-2016 Free Software Foundation, Inc.
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
#include <math_ldbl_opt.h>

	.section	.rodata.cst4,"aM",@progbits,4
	.align	2
.LC0:	/* 0.5 */
	.long 0x3f000000
.LC1:	/* 2^52.  */
	.long 0x59800000
	.section	.rodata.cst8,"aM",@progbits,8
	.align	3
.LC2:	/* 0x7fffffff.8p0.  */
	.long 0x41dfffff
	.long 0xffe00000
.LC3:	/* -0x80000000.8p0.  */
	.long 0xc1e00000
	.long 0x00100000
	.section	".text"

/* long [r3] lround (float x [fp1])
   IEEE 1003.1 lround function.  IEEE specifies "round to the nearest
   integer value, rounding halfway cases away from zero, regardless of
   the current rounding mode."  However PowerPC Architecture defines
   "round to Nearest" as "Choose the best approximation. In case of a
   tie, choose the one that is even (least significant bit o).".
   So we can't use the PowerPC "round to Nearest" mode. Instead we set
   "round toward Zero" mode and round by adding +-0.5 before rounding
   to the integer value.  It is necessary to detect when x is
   (+-)0x1.fffffffffffffp-2 because adding +-0.5 in this case will
   cause an erroneous shift, carry and round.  We simply return 0 if
   0.5 > x > -0.5.  */

ENTRY (__lround)
	stwu    r1,-16(r1)
	cfi_adjust_cfa_offset (16)
#ifdef SHARED
	mflr	r11
	cfi_register(lr,r11)
	SETUP_GOT_ACCESS(r9,got_label)
	addis	r10,r9,.LC0-got_label@ha
	lfs	fp10,.LC0-got_label@l(r10)
	addis	r10,r9,.LC1-got_label@ha
	lfs	fp11,.LC1-got_label@l(r10)
	addis	r10,r9,.LC2-got_label@ha
	lfd	fp9,.LC2-got_label@l(r10)
	addis	r10,r9,.LC3-got_label@ha
	lfd	fp8,.LC3-got_label@l(r10)
	mtlr	r11
	cfi_same_value (lr)
#else
	lis	r9,.LC0@ha
	lfs	fp10,.LC0@l(r9)
	lis	r9,.LC1@ha
	lfs	fp11,.LC1@l(r9)
	lis	r9,.LC2@ha
	lfd	fp9,.LC2@l(r9)
	lis	r9,.LC3@ha
	lfd	fp8,.LC3@l(r9)
#endif
	fabs	fp2, fp1	/* Get the absolute value of x.  */
	fsub	fp12,fp10,fp10	/* Compute 0.0.  */
	fcmpu	cr6, fp2, fp10	/* if |x| < 0.5  */
	fcmpu	cr5, fp1, fp9	/* if x >= 0x7fffffff.8p0  */
	fcmpu	cr1, fp1, fp8	/* if x <= -0x80000000.8p0  */
	fcmpu	cr7, fp1, fp12	/* x is negative? x < 0.0  */
	blt-	cr6,.Lretzero
	bge-	cr5,.Loflow
	ble-	cr1,.Loflow
	/* Test whether an integer to avoid spurious "inexact".  */
	fadd	fp3,fp2,fp11
	fsub	fp3,fp3,fp11
	fcmpu	cr5, fp2, fp3
	beq	cr5,.Lnobias
	fadd	fp3,fp2,fp10	/* |x|+=0.5 bias to prepare to round.  */
	bge	cr7,.Lconvert	/* x is positive so don't negate x.  */
	fnabs	fp3,fp3		/* -(|x|+=0.5)  */
.Lconvert:
	fctiwz	fp4,fp3		/* Convert to Integer word lround toward 0.  */
	stfd	fp4,8(r1)
	nop	/* Ensure the following load is in a different dispatch  */
	nop	/* group to avoid pipe stall on POWER4&5.  */
	nop
	lwz	r3,8+LOWORD(r1)	/* Load return as integer.  */
.Lout:
	addi	r1,r1,16
	blr
.Lretzero:			/* when 0.5 > x > -0.5  */
	li	r3,0		/* return 0.  */
	b	.Lout
.Lnobias:
	fmr	fp3,fp1
	b	.Lconvert
.Loflow:
	fmr	fp3,fp11
	bge	cr7,.Lconvert
	fnabs	fp3,fp3
	b	.Lconvert
	END (__lround)

weak_alias (__lround, lround)

strong_alias (__lround, __lroundf)
weak_alias (__lround, lroundf)

#ifdef NO_LONG_DOUBLE
weak_alias (__lround, lroundl)
strong_alias (__lround, __lroundl)
#endif
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __lround, lroundl, GLIBC_2_1)
#endif
