/* Round long double to long int.
   IBM extended format long double version.
   Copyright (C) 2004,2006 Free Software Foundation, Inc.
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
.LC0:	/* 2**52 */
	.tc FD_43300000_0[TC],0x4330000000000000
.LC1:	/* 2**63 */
	.tc FD_43E00000_0[TC],0x43e0000000000000
	.section	".text"

/* long long int[r3] __llrintl (long double x[fp1,fp2])  */
ENTRY (__llrintl)
	lfd	fp13,.LC0@toc(2)
	lfd	fp10,.LC1@toc(2)
	fabs	fp0,fp1
	fcmpu	cr7,fp0,fp13	/* if (fabs(x) > TWO52)  */
	fcmpu	cr6,fp0,fp10	/* if (fabs(x) > TWO63)  */
	beq-	cr6,.L2
	fctid	fp11,fp1	/* must delay this opperation to here  */
	fctid	fp12,fp2	/* and avoid setting "invalid operation".  */
	li	r0,0
	stfd	fp11,-16(r1)
	bgt-	cr6,.L9		/* if > TWO63 return  "invalid operation".  */
	ble+	cr7,.L9		/* If < TWO52 only thy high double is used.  */
	stfd	fp12,-8(r1)
	nop	/* Insure the following load is in a different dispatch group */
	nop	/* to avoid pipe stall on POWER4&5.  */
	nop
.L8:
	ld	r0,-8(r1)
.L9:
	ld	r3,-16(r1)
	add	r3,r3,r0
	blr

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
	fctid	fp11,fp1	/* Otherwise we want to set "invalid operation".  */
	li	r0,0
	stfd	fp11,-16(r1)
	b	.L9

END (__llrintl)

strong_alias (__llrintl, __lrintl)
long_double_symbol (libm, __llrintl, llrintl)
long_double_symbol (libm, __lrintl, lrintl)
