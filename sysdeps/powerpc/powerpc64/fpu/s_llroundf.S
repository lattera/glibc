/* llroundf function.  PowerPC64 version.
   Copyright (C) 2004, 2006, 2007, 2008 Free Software Foundation, Inc.
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

	.section	".toc","aw"
.LC0:	/* 2^23 */
	.tc FD_41600000_0[TC],0x4160000000000000
.LC1:	/* 0.5 */
	.tc FD_3fe00000_0[TC],0x3fe0000000000000
	.section	".text"
	
/* long long [r3] llroundf (float x [fp1])
   IEEE 1003.1 llroundf function.  IEEE specifies "roundf to the nearest 
   integer value, rounding halfway cases away from zero, regardless of
   the current rounding mode."  However PowerPC Architecture defines
   "roundf to Nearest" as "Choose the best approximation. In case of a 
   tie, choose the one that is even (least significant bit o).". 
   So we can't use the PowerPC "round to Nearest" mode. Instead we set
   "round toward Zero" mode and round by adding +-0.5 before rounding
   to the integer value.

   It is necessary to detect when x is (+-)0x1.fffffffffffffp-2
   because adding +-0.5 in this case will cause an erroneous shift,
   carry and round.  We simply return 0 if 0.5 > x > -0.5.  Likewise
   if x is and odd number between +-(2^23 and 2^24-1) a shift and
   carry will erroneously round if biased with +-0.5.  Therefore if x
   is greater/less than +-2^23 we don't need to bias the number with
   +-0.5.  */

ENTRY (__llroundf)
	CALL_MCOUNT 0
	lfd	fp9,.LC0@toc(2)	/* Load 2^23 into fpr9.  */
	lfd	fp10,.LC1@toc(2)/* Load 0.5 into fpr10.  */
	fabs	fp2,fp1		/* Get the absolute value of x.  */
	fsub	fp12,fp10,fp10	/* Compute 0.0 into fp12.  */
	fcmpu	cr6,fp2,fp10	/* if |x| < 0.5  */
	fcmpu	cr7,fp2,fp9	/* if |x| >= 2^23  */
	fcmpu	cr1,fp1,fp12	/* x is negative? x < 0.0  */
	blt-	cr6,.Lretzero	/* 0.5 > x < -0.5 so just return 0.  */
	bge-	cr7,.Lnobias	/* 2^23 > x < -2^23 just convert with no bias.  */
	fadd	fp3,fp2,fp10	/* |x|+=0.5 bias to prepare to round.  */
	bge	cr1,.Lconvert	/* x is positive so don't negate x.  */
	fnabs	fp3,fp3		/* -(|x|+=0.5)  */
.Lconvert:
	fctidz	fp4,fp3		/* Convert to Integer double word round toward 0.  */
	stfd	fp4,-16(r1)
	nop
	nop
	nop
	ld	r3,-16(r1)	/* Load return as integer.  */
.Lout:
	blr
.Lretzero:			/* 0.5 > x > -0.5  */
	li	r3,0		/* return 0.  */
	b	.Lout
.Lnobias:
	fmr	fp3,fp1
	b	.Lconvert
	END (__llroundf)

strong_alias (__llroundf, __lroundf)
weak_alias (__llroundf, llroundf)
weak_alias (__lroundf, lroundf)

