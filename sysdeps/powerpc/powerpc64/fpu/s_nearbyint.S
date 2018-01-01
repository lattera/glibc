/* Round to int floating-point values.  PowerPC64 version.
   Copyright (C) 2011-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Adhemerval Zanella <azanella@br.ibm.com>, 2011

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

/* This has been coded in assembler because GCC makes such a mess of it
   when it's coded in C.  */

#include <sysdep.h>
#include <math_ldbl_opt.h>
#include <libm-alias-double.h>


/* double [fp1] nearbyint(double [fp1] x) */

	.section	".toc","aw"
.LC0:	/* 2**52 */
	.tc FD_43300000_0[TC],0x4330000000000000
	.section	".text"

ENTRY (__nearbyint, 4)
	CALL_MCOUNT 0
	fabs	fp0,fp1
	lfd	fp13,.LC0@toc(2)
	fcmpu	cr7,fp0,fp13	/* if (fabs(x) > TWO52)  */
	bge	cr7,.L10
	fsub	fp12,fp13,fp13	/* generate 0.0 */
	fcmpu	cr7,fp1,fp12	/* if (x > 0.0) */
	ble	cr7, L(lessthanzero)
	mffs	fp11
	mtfsb0	4*cr7+lt	/* Disable FE_INEXACT exception */
	fadd	fp1,fp1,fp13	/* x+= TWO52 */
	fsub	fp1,fp1,fp13	/* x-= TWO52 */
	fabs	fp1,fp1		/* if (x == 0.0) */
	mtfsf	0xff,fp11	/* Restore FE_INEXACT state.  */
	blr			/* x = 0.0; */
L(lessthanzero):
	bgelr	cr7		/* if (x < 0.0) */
	mffs	fp11
	mtfsb0	4*cr7+lt
	fsub	fp1,fp1,fp13	/* x -= TWO52 */
	fadd	fp1,fp1,fp13	/* x += TWO52 */
	fnabs	fp1,fp1		/* if (x == 0.0) */
	mtfsf	0xff,fp11	/* Restore FE_INEXACT state.  */
	blr			/* x = -0.0; */
.L10:
	/* Ensure sNaN input is converted to qNaN.  */
	fcmpu	cr7,fp1,fp1
	beqlr	cr7
	fadd	fp1,fp1,fp1
	blr
END (__nearbyint)

libm_alias_double (__nearbyint, nearbyint)
