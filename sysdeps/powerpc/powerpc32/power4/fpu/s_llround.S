/* llround function.  PowerPC32 on PowerPC64 version.
   Copyright (C) 2004-2015 Free Software Foundation, Inc.
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

 .section .rodata.cst8,"aM",@progbits,8
 .align 3
 .LC0:	.long (52+127)<<23 /* 0x1p+52  */
	.long (-1+127)<<23 /* 0.5  */

	.section	".text"

/* long [r3] lround (float x [fp1])
   IEEE 1003.1 lround function.  IEEE specifies "round to the nearest
   integer value, rounding halfway cases away from zero, regardless of
   the current rounding mode."  However PowerPC Architecture defines
   "round to Nearest" as "Choose the best approximation. In case of a
   tie, choose the one that is even (least significant bit o).".
   So we can't use the PowerPC "round to Nearest" mode. Instead we set
   "round toward Zero" mode and round by adding +-0.5 before rounding
   to the integer value.

   It is necessary to detect when x is (+-)0x1.fffffffffffffp-2
   because adding +-0.5 in this case will cause an erroneous shift,
   carry and round.  We simply return 0 if 0.5 > x > -0.5.  Likewise
   if x is and odd number between +-(2^52 and 2^53-1) a shift and
   carry will erroneously round if biased with +-0.5.  Therefore if x
   is greater/less than +-2^52 we don't need to bias the number with
   +-0.5.  */

ENTRY (__llround)
	stwu    r1,-16(r1)
	cfi_adjust_cfa_offset (16)
#ifdef SHARED
	mflr	r11
	cfi_register(lr,r11)
	SETUP_GOT_ACCESS(r9,got_label)
	addis	r9,r9,.LC0-got_label@ha
	addi	r9,r9,.LC0-got_label@l
	mtlr	r11
	cfi_same_value (lr)
	lfs	fp9,0(r9)
	lfs	fp10,4(r9)
#else
	lis r9,.LC0@ha
	lfs fp9,.LC0@l(r9)	/* Load 2^52 into fpr9.  */
	lfs fp10,.LC0@l+4(r9)	/* Load 0.5 into fpr10.  */
#endif
	fabs	fp2,fp1		/* Get the absolute value of x.  */
	fsub	fp12,fp10,fp10	/* Compute 0.0 into fpr12.  */
	fcmpu	cr6,fp2,fp10	/* if |x| < 0.5  */
	fcmpu	cr7,fp2,fp9	/* if |x| >= 2^52  */
	fcmpu	cr1,fp1,fp12	/* x is negative? x < 0.0  */
	blt-	cr6,.Lretzero	/* 0.5 > x < -0.5 so just return 0.  */
	bge-	cr7,.Lnobias	/* 2^52 > x < -2^52 just convert with no bias.  */
	fadd	fp3,fp2,fp10	/* |x|+=0.5 bias to prepare to round.  */
	bge	cr1,.Lconvert	/* x is positive so don't negate x.  */
	fnabs	fp3,fp3		/* -(|x|+=0.5)  */
.Lconvert:
	fctidz	fp4,fp3		/* Convert to Integer double word round toward 0.  */
	stfd	fp4,8(r1)
	nop
	nop
	nop
	lwz	r3,8+HIWORD(r1)	/* Load return as integer.  */
	lwz	r4,8+LOWORD(r1)
.Lout:
	addi	r1,r1,16
	blr
.Lretzero:			/* 0.5 > x > -0.5  */
	li	r3,0		/* return 0.  */
	li	r4,0
	b	.Lout
.Lnobias:
	fmr	fp3,fp1
	b	.Lconvert
	END (__llround)

weak_alias (__llround, llround)

strong_alias (__llround, __llroundf)
weak_alias (__llround, llroundf)

#ifdef NO_LONG_DOUBLE
weak_alias (__llround, llroundl)
strong_alias (__llround, __llroundl)
#endif
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __llround, llroundl, GLIBC_2_1)
#endif
