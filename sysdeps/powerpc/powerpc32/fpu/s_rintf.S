/* Round float to int floating-point values.  PowerPC32 version.
   Copyright (C) 2004 Free Software Foundation, Inc.
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


	.section	.rodata
	.align 3
	.type	TWO23.0,@object
	.size	TWO23.0,8
TWO23.0:
	.long 0x41600000
	.long 0

	.section	.rodata.cst8,"aM",@progbits,8
	.align 3
.LC0:	/* 2**23 */
	.long 0x41600000
	.long 0

	.section	".text"
ENTRY (__rintf)
#ifdef SHARED
	mflr	r11
	bl	_GLOBAL_OFFSET_TABLE_@local-4
	mflr	r10
	lwz	r9,.LC0@got(10)
	mtlr	r11
	lfd	fp13,0(r9)
#else
	lis	r9,.LC0@ha
	lfd	fp13,.LC0@l(r9)
#endif
	fabs	fp0,fp1
	fsubs	fp12,fp13,fp13	/* generate 0.0  */
	fcmpu	cr7,fp0,fp13	/* if (fabs(x) > TWO23)  */
	fcmpu	cr6,fp1,fp12	/* if (x > 0.0)  */
	bnllr-	cr7
	bng-	cr6,.L4
	fadds	fp1,fp1,fp13	/* x+= TWO23;  */
	fsubs	fp1,fp1,fp13	/* x-= TWO23;  */
	blr
.L4:
	bnllr-	cr6		/* if (x < 0.0)  */
	fsubs	fp1,fp13,fp1	/* x = TWO23 - x;  */
	fsubs	fp0,fp1,fp13	/* x = - (x - TWO23);  */
	fneg	fp1,fp0
	blr
	END (__rintf)

weak_alias (__rintf, rintf)

