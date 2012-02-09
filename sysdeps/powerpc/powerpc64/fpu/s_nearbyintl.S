/* nearbyint long double.
   IBM extended format long double version.
   Copyright (C) 2004, 2006 Free Software Foundation, Inc.
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

	.section	".toc","aw"
.LC0:	/* 2**52 */
	.tc FD_43300000_0[TC],0x4330000000000000
	.section	".text"

/* long double [fp1,fp2] nearbyintl (long double x [fp1,fp2])
   IEEE 1003.1 nearbyintl function.  nearbyintl is simular to the rintl
   but does raise the "inexact" exception.  This implementation is
   based on rintl but explicitly maskes the inexact exception on entry
   and clears any pending inexact before restoring the exception mask
   on exit.

   PowerPC64 long double uses the IBM extended format which is
   represented two 64-floating point double values. The values are
   non-overlapping giving an effective precision of 106 bits. The first
   double contains the high order bits of mantisa and is always rounded
   to represent a normal rounding of long double to double. Since the
   long double value is sum of the high and low values, the low double
   normally has the opposite sign to compensate for the this rounding.

   For long double there are two cases:
   1) |x| < 2**52, all the integer bits are in the high double.
      floor the high double and set the low double to -0.0.
   2) |x| >= 2**52, Rounding involves both doubles.
      See the comment before lable .L2 for details.
   */
ENTRY (__nearbyintl)
	mffs	fp11		/* Save current FPSCR.  */
	lfd	fp13,.LC0@toc(2)
	fabs	fp0,fp1
	mtfsb0  28		/* Disable "inexact" exceptions.  */
	fsub	fp12,fp13,fp13	/* generate 0.0  */
	fabs	fp9,fp2
	fcmpu	cr7,fp0,fp13	/* if (fabs(x) > TWO52)  */
	fcmpu	cr6,fp1,fp12	/* if (x > 0.0)  */
	bnl-	cr7,.L2
	fmr	fp2,fp12
	bng-	cr6,.L4
	fadd	fp1,fp1,fp13	/* x+= TWO52;  */
	fsub	fp1,fp1,fp13	/* x-= TWO52;  */
	b	.L9
.L4:
	bnl-	cr6,.L9		/* if (x < 0.0)  */
	fsub	fp1,fp13,fp1	/* x = TWO52 - x;  */
	fsub	fp0,fp1,fp13	/* x = - (x - TWO52);  */
	fneg	fp1,fp0
.L9:
	mtfsb0	6		/* Clear any pending "inexact" exceptions.  */
	mtfsf	0x01,fp11	/* restore exception mask.  */
	blr

/* The high double is > TWO52 so we need to round the low double and
   perhaps the high double.  This gets a bit tricky so we use the
   following algorithm:

   tau = floor(x_high/TWO52);
   x0 = x_high - tau;
   x1 = x_low + tau;
   r1 = nearbyint(x1);
   y_high = x0 + r1;
   y_low = r1 - tau;
   return y;  */
.L2:
	fcmpu	cr7,fp9,fp13	/* if (|x_low| > TWO52)  */
	fcmpu	cr0,fp9,fp12	/* || (|x_low| == 0.0)  */
	bge-	cr7,.L9		/*   return x;	*/
	beq-  cr0,.L9
	fdiv	fp8,fp1,fp13	/* x_high/TWO52  */
	fctidz	fp0,fp8
	fcfid	fp8,fp0		/* tau = floor(x_high/TWO52);  */
	fsub	fp3,fp1,fp8	/* x0 = x_high - tau;  */
	fadd	fp4,fp2,fp8	/* x1 = x_low + tau;  */

	fcmpu	cr6,fp4,fp12	/* if (x1 > 0.0)  */
	bng-	cr6,.L8
	fadd	fp5,fp4,fp13	/* r1 = x1 + TWO52;  */
	fsub	fp5,fp5,fp13	/* r1 = r1 - TWO52;  */
	b	.L6
.L8:
	fmr	fp5,fp4
	bge-	cr6,.L6		/* if (x1 < 0.0)  */
	fsub	fp5,fp13,fp4	/* r1 = TWO52 - x1;  */
	fsub	fp0,fp5,fp13	/* r1 = - (r1 - TWO52);  */
	fneg	fp5,fp0
.L6:
	fadd	fp1,fp3,fp5	/* y_high = x0 + r1;  */
	fsub	fp2,fp5,fp8	/* y_low = r1 - tau;  */
	b	.L9
END (__nearbyintl)

long_double_symbol (libm, __nearbyintl, nearbyintl)
