/* llroundl function.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include <math_ldbl_opt.h>

	.section	".toc","aw"
.LC0:	/* 0.0 */
	.tc FD_00000000_0[TC],0x0000000000000000
.LC1:	/* 0.5 */
	.tc FD_3fe00000_0[TC],0x3fe0000000000000
.LC2:	/* 2**52 */
	.tc FD_43300000_0[TC],0x4330000000000000
.LC3:	/* 2**63 */
	.tc FD_43E00000_0[TC],0x43e0000000000000
	.section	".text"

/* long long [r3] llround (long double x [fp1,fp2])
   IEEE 1003.1 llroundl function.  IEEE specifies "round to the nearest
   integer value, rounding halfway cases away from zero, regardless of
   the current rounding mode."  However PowerPC Architecture defines
   "round to Nearest" as "Choose the best approximation. In case of a
   tie, choose the one that is even (least significant bit o).".
   So we can't use the PowerPC "round to Nearest" mode. Instead we set
   "round toward Zero" mode and round by adding +-0.5 before rounding
   toward zero. The "Floating Convert To Integer Doubleword with round
   toward zero" instruction handles the conversion including the
   overflow cases and signalling "Invalid Operation".

   PowerPC64 long double uses the IBM extended format which is
   represented two 64-floating point double values. The values are
   non-overlapping giving an effective precision of 106 bits. The first
   double contains the high order bits of mantisa and is always rounded
   to represent a normal rounding of long double to double. Since the
   long double value is sum of the high and low values, the low double
   normally has the opposite sign to compensate for the this rounding.

   For long double there is 4 cases:
   1) |x| < 2**52, all the integer bits are in the high double.
      Round and convert the high double to long long.
   2) 2**52 <= |x|< 2**63, Still fits but need bits from both doubles.
      Round the low double, convert both, then sum the long long values.
   3) |x| == 2**63, Looks like an overflow but may not be due to rounding
      of the high double.
      See the description following lable L2.
   4) |x| > 2**63, This will overflow the 64-bit signed integer.
      Treat like case #1. The fctidz instruction will generate the
      appropriate and signal "invalid operation".

   */

ENTRY (__llroundl)
	fabs	fp0,fp1
	lfd	fp13,.LC2@toc(2)	/* 2**52 */
	lfd	fp12,.LC3@toc(2)	/* 2**63 */
	lfd	fp11,.LC0@toc(2)	/* 0.0 */
	lfd	fp10,.LC1@toc(2)	/* 0.5 */
	fcmpu	cr0,fp0,fp12		/* if (x < TWO63 */
	fcmpu	cr7,fp0,fp13		/* if (x < TWO52 */
	fcmpu	cr6,fp1,fp11		/* if (x > 0.0)  */
	bge-	cr0,.L2
	bge-	cr7,.L8
	ble-	cr6,.L4
	fadd	fp4,fp2,fp10		/* x+= 0.5;  */
	fadd	fp5,fp1,fp4		/* x+= 0.5;  */
.L9:
	fctidz	fp3,fp5		/* Convert To Integer DW llround toward 0.  */
	stfd	fp3,-16(r1)
	nop	/* Insure the following load is in a different dispatch group */
	nop	/* to avoid pipe stall on POWER4&5.  */
	nop
	ld	r3,-16(r1)
	blr
.L4:
	fsub	fp4,fp2,fp10	/* x-= 0.5;  */
	fadd	fp5,fp1,fp4		/* x+= 0.5;  */
	b	.L9
.L8:
	ble	cr6,.L6
	fneg	fp10,fp10
.L6:
	fadd	fp2,fp2,fp10
	fctidz	fp3,fp1		/* Convert To Integer DW llround toward 0.  */
	fctidz	fp4,fp2		/* Convert To Integer DW llround toward 0.  */
	stfd	fp3,-16(r1)
	stfd	fp4,-8(r1)
	nop	/* Insure the following load is in a different dispatch group */
	nop	/* to avoid pipe stall on POWER4&5.  */
	nop
	ld	r3,-16(r1)
	ld	r0,-8(r1)
	add	r3,r3,r0
	blr
.L2:
/* The high double is >= TWO63 so it looks like we are "out of range".
   But this may be caused by rounding of the high double and the
   negative low double may bring it back into range. So we need to
   de-round the high double and invert the low double without changing
   the effective long double value. To do this we compute a special
   value (tau) that we can subtract from the high double and add to
   the low double before conversion. The resulting integers can be
   summed to get the total value.

   tau = floor(x_high/TWO52);
   x0 = x_high - tau;
   x1 = x_low + tau;  */
.L2:
	fdiv	fp8,fp1,fp13	/* x_high/TWO52  */
	bgt-	cr0,.L9		/* if x > TWO63  */
	fctidz	fp0,fp8
	fcfid	fp8,fp0		/* tau = floor(x_high/TWO52);  */
	fsub	fp3,fp1,fp8	/* x0 = x_high - tau;  */
	fadd	fp4,fp2,fp8	/* x1 = x_low + tau;  */
	fctid	fp11,fp3
	fctid	fp12,fp4
	stfd	fp11,-16(r1)
	stfd	fp12,-8(r1)
	nop	/* Insure the following load is in a different dispatch group */
	nop	/* to avoid pipe stall on POWER4&5.  */
	nop
	ld	r3,-16(r1)
	ld	r0,-8(r1)
	addo.	r3,r3,r0
	bnslr+  cr0		/* if the sum does not overflow, return.  */
	b	.L9		/* Otherwise we want to set "invalid operation".  */
END (__llroundl)

strong_alias (__llroundl, __lroundl)
long_double_symbol (libm, __llroundl, llroundl)
long_double_symbol (libm, __lroundl, lroundl)
