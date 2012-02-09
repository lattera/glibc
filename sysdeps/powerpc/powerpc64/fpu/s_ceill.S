/* s_ceill.S IBM extended format long double version.
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

/* long double [fp1,fp2] ceill (long double x [fp1,fp2])
   IEEE 1003.1 ceil function.

   PowerPC64 long double uses the IBM extended format which is
   represented two 64-floating point double values. The values are
   non-overlapping giving an effective precision of 106 bits. The first
   double contains the high order bits of mantisa and is always ceiled
   to represent a normal ceiling of long double to double. Since the
   long double value is sum of the high and low values, the low double
   normally has the opposite sign to compensate for the this ceiling.

   For long double there are two cases:
   1) |x| < 2**52, all the integer bits are in the high double.
      ceil the high double and set the low double to -0.0.
   2) |x| >= 2**52, ceiling involves both doubles.
      See the comment before lable .L2 for details.
   */

ENTRY (__ceill)
	mffs	fp11		/* Save current FPU rounding mode.  */
	lfd	fp13,.LC0@toc(2)
	fabs	fp0,fp1
	fabs	fp9,fp2
	fsub	fp12,fp13,fp13	/* generate 0.0  */
	fcmpu	cr7,fp0,fp13	/* if (fabs(x) > TWO52)  */
	fcmpu	cr6,fp1,fp12	/* if (x > 0.0)  */
	bnl-	cr7,.L2
	mtfsfi	7,2		/* Set rounding mode toward +inf.  */
	fneg	fp2,fp12
	ble-	cr6,.L1
	fadd	fp1,fp1,fp13	/* x+= TWO52;  */
	fsub	fp1,fp1,fp13	/* x-= TWO52;  */
	fabs	fp1,fp1		/* if (x == 0.0)  */
.L0:
	mtfsf	0x01,fp11	/* restore previous rounding mode.  */
	blr			/* x = 0.0; */
.L1:
	bge-	cr6,.L0		/* if (x < 0.0)  */
	fsub	fp1,fp1,fp13	/* x-= TWO52;  */
	fadd	fp1,fp1,fp13	/* x+= TWO52;  */
	fcmpu	cr5,fp1,fp12	/* if (x > 0.0)  */
	mtfsf	0x01,fp11	/* restore previous rounding mode.  */
	fnabs	fp1,fp1		/* if (x == 0.0)  */
	blr			/* x = -0.0; */
	
/* The high double is > TWO52 so we need to round the low double and
   perhaps the high double.  In this case we have to round the low
   double and handle any adjustment to the high double that may be
   caused by rounding (up).  This is complicated by the fact that the
   high double may already be rounded and the low double may have the
   opposite sign to compensate.This gets a bit tricky so we use the
   following algorithm:

   tau = floor(x_high/TWO52);
   x0 = x_high - tau;
   x1 = x_low + tau;
   r1 = rint(x1);
   y_high = x0 + r1;
   y_low = x0 - y_high + r1;
   return y;  */
.L2:
	fcmpu	cr7,fp9,fp13	/* if (|x_low| > TWO52)  */
	fcmpu	cr0,fp9,fp12	/* || (|x_low| == 0.0)  */
	fcmpu	cr5,fp2,fp12	/* if (x_low > 0.0)  */
	bgelr-	cr7		/*   return x;	*/
	beqlr-  cr0
	mtfsfi	7,2		/* Set rounding mode toward +inf.  */
	fdiv	fp8,fp1,fp13	/* x_high/TWO52  */
	
	bng-	cr6,.L6		/* if (x > 0.0)  */
	fctidz	fp0,fp8
	fcfid	fp8,fp0		/* tau = floor(x_high/TWO52);  */
	bng	cr5,.L4		/* if (x_low > 0.0)  */
	fmr	fp3,fp1
	fmr	fp4,fp2
	b	.L5
.L4:				/* if (x_low < 0.0)  */
	fsub	fp3,fp1,fp8	/* x0 = x_high - tau;  */
	fadd	fp4,fp2,fp8	/* x1 = x_low + tau;  */
.L5:
	fadd	fp5,fp4,fp13	/* r1 = r1 + TWO52;  */
	fsub	fp5,fp5,fp13	/* r1 = r1 - TWO52;  */
	b	.L9
.L6:				/* if (x < 0.0)  */
	fctidz	fp0,fp8
	fcfid	fp8,fp0		/* tau = floor(x_high/TWO52);  */	
	bnl	cr5,.L7		/* if (x_low < 0.0)  */
	fmr	fp3,fp1
	fmr	fp4,fp2
	b	.L8
.L7:				/* if (x_low > 0.0)  */
	fsub	fp3,fp1,fp8	/* x0 = x_high - tau;  */
	fadd	fp4,fp2,fp8	/* x1 = x_low + tau;  */
.L8:
	fsub	fp5,fp4,fp13	/* r1-= TWO52;  */
	fadd	fp5,fp5,fp13	/* r1+= TWO52;  */
.L9:
	mtfsf	0x01,fp11	/* restore previous rounding mode.  */
	fadd	fp1,fp3,fp5	/* y_high = x0 + r1;  */
	fsub	fp2,fp3,fp1	/* y_low = x0 - y_high + r1;  */
	fadd	fp2,fp2,fp5
	blr
END (__ceill)

long_double_symbol (libm, __ceill, ceill)
