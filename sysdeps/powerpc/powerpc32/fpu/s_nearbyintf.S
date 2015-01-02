/* Round to int floating-point values.  PowerPC32 version.
   Copyright (C) 2011-2015 Free Software Foundation, Inc.
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


/* float [fp1] nearbyintf(float [fp1] x) */

	.section	.rodata.cst4,"aM",@progbits,4
	.align	2
.LC0:
	.long 0x4B000000	/* TWO23: 2**23 */

	.section	".text"
ENTRY (__nearbyintf)
#ifdef SHARED
	mflr	r11
	cfi_register(lr,r11)
	bcl	20,31,1f
1:	mflr	r9
	addis	r9,r9,.LC0-1b@ha
	lfs	fp13,.LC0-1b@l(r9)
	mtlr	r11
	cfi_same_value (lr)
#else
	lis	r9,.LC0@ha
	lfs	fp13,.LC0@l(r9)
#endif
	fabs	fp0,fp1
	fsub    fp12,fp13,fp13		/* generate 0.0  */
	fcmpu	cr7,fp0,fp13		/* if (fabs(x) > TWO23 */
	bgelr	cr7
	fcmpu	cr7,fp1,fp12		/* if (x > 0.0 */
	ble	cr7,L(lessthanzero)
	mtfsb0	4*cr7+lt		/* Disable FE_INEXACT exception */
	fadds	fp1,fp1,fp13		/* x += TWO23 */
	fsubs	fp1,fp1,fp13		/* x -= TWO23 */
	fabs	fp1,fp1			/* if (x == 0.0) */
	mtfsb0	4*cr1+eq		/* Clear any FE_INEXACT exception */
	blr
L(lessthanzero):
	bgelr	cr7
	mtfsb0	4*cr7+lt		/* Disable FE_INEXACT exception */
	fsubs	fp1,fp1,fp13		/* x -= TWO23 */
	fadds	fp1,fp1,fp13		/* x += TWO23 */
	fnabs	fp1,fp1			/* if (x == 0.0) */
	mtfsb0	4*cr1+eq		/* Clear any FE_INEXACT exception */
	blr
END (__nearbyintf)

weak_alias (__nearbyintf, nearbyintf)
