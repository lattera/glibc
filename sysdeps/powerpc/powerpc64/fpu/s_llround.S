/* llround function.  PowerPC64 version.
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

	.section	".toc","aw"
.LC0:	/* -0.0 */
	.tc FD_00000000_0[TC],0x0000000000000000
.LC1:	/* 0.5 */
	.tc FD_3fe00000_0[TC],0x3fe0000000000000
	.section	".text"
	
/* long long [r3] llround (float x [fp1])
   IEEE 1003.1 llround function.  IEEE specifies "round to the nearest 
   integer value, rounding halfway cases away from zero, regardless of
   the current rounding mode."  However PowerPC Architecture defines
   "round to Nearest" as "Choose the best approximation. In case of a 
   tie, choose the one that is even (least significant bit o).". 
   So we can't use the PowerPC "round to Nearest" mode. Instead we set
   "round toward Zero" mode and round by adding +-0.5 before rounding
   to the integer value.  */

ENTRY (__llround)
	CALL_MCOUNT 0
	lfd	fp12,.LC0@toc(2)
	lfd	fp10,.LC1@toc(2)
	fcmpu	cr6,fp1,fp12	/* if (x > 0.0)  */
	ble-	cr6,.L4
	fadd	fp1,fp1,fp10	/* x+= 0.5;  */
.L9:
	fctidz	fp2,fp1		/* Convert To Integer DW llround toward 0.  */
	stfd	fp2,-16(r1)
	nop	/* Insure the following load is in a different dispatch group */
	nop	/* to avoid pipe stall on POWER4&5.  */
	nop
	ld	r3,-16(r1)
	blr
.L4:
	fsub	fp1,fp1,fp10	/* x-= 0.5;  */
	b	.L9
	END (__llround)

strong_alias (__llround, __lround)
weak_alias (__llround, llround)
weak_alias (__lround, lround)

#ifdef NO_LONG_DOUBLE
weak_alias (__llround, llroundl)
strong_alias (__llround, __llroundl)
weak_alias (__lround, lroundl)
strong_alias (__lround, __lroundl)
#endif
