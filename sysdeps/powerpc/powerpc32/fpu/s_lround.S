/* lround function.  PowerPC32 version.
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
	.type	NEGZERO.0,@object
	.size	NEGZERO.0,8
NEGZERO.0:
	.long 0x00000000
	.long 0
	.type	POINTFIVE.0,@object
	.size	POINTFIVE.0,8
POINTFIVE.0:
	.long 0x3fe00000
	.long 0

	.section	.rodata.cst8,"aM",@progbits,8
	.align 3
.LC0:	/* 0.0 */
	.long 0x00000000
	.long 0
.LC1:	/* 0.5 */
	.long 0x3fe00000
	.long 0

	.section	".text"
	
/* long [r3] lround (float x [fp1])
   IEEE 1003.1 lround function.  IEEE specifies "round to the nearest 
   integer value, rounding halfway cases away from zero, regardless of
   the current rounding mode."  However PowerPC Architecture defines
   "round to Nearest" as "Choose the best approximation. In case of a 
   tie, choose the one that is even (least significant bit o).". 
   So we can't use the PowerPC "round to Nearest" mode. Instead we set
   "round toward Zero" mode and round by adding +-0.5 before rounding
   to the integer value.  */

ENTRY (__lround)
#ifdef SHARED
	mflr	r11
	bl	_GLOBAL_OFFSET_TABLE_@local-4
	mflr	r10
	lwz	r9,.LC0@got(10)
	mtlr	r11
	lfd	fp12,0(r9)
#else
	lis	r9,.LC0@ha
	lfd	fp12,.LC0@l(r9)
#endif
#ifdef SHARED
	lwz	r9,.LC1@got(10)
	lfd	fp10,0(r9)
#else
	lis	r9,.LC1@ha
	lfd	fp10,.LC1@l(r9)
#endif
	fcmpu	cr6,fp1,fp12	/* if (x > 0.0)  */
	ble-	cr6,.L4
	fadd	fp1,fp1,fp10	/* x+= 0.5;  */
.L9:
	fctiwz	fp2,fp1		/* Convert To Integer DW lround toward 0.  */
	stfd	fp2,-8(r1)
	nop	/* Insure the following load is in a different dispatch group */
	nop	/* to avoid pipe stall on POWER4&5.  */
	nop
	lwz	r3,-4(r1)
	blr
.L4:
	fsub	fp1,fp1,fp10	/* x-= 0.5;  */
	b	.L9
	END (__lround)

strong_alias (__lround, __lround)
weak_alias (__lround, lround)

strong_alias (__lround, __lroundf)
weak_alias (__lround, lroundf)

#ifdef NO_LONG_DOUBLE
weak_alias (__lround, lroundl)
strong_alias (__lround, __lroundl)
#endif
