/* round function.  PowerPC64 version.
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
.LC0:	/* 2**52 */
	.tc FD_43300000_0[TC],0x4330000000000000
.LC1:	/* 0.5 */
	.tc FD_3fe00000_0[TC],0x3fe0000000000000
	.section	".text"
	
/* double [fp1] round (double x [fp1])
   IEEE 1003.1 round function.  IEEE specifies "round to the nearest 
   integer value, rounding halfway cases away from zero, regardless of
   the current rounding mode."  However PowerPC Architecture defines
   "Round to Nearest" as "Choose the best approximation. In case of a 
   tie, choose the one that is even (least significant bit o).". 
   So we can't use the PowerPC "Round to Nearest" mode. Instead we set
   "Round toward Zero" mode and round by adding +-0.5 before rounding
   to the integer value.  */

EALIGN (__round, 4, 0)
	CALL_MCOUNT 0
	mffs	fp11		/* Save current FPU rounding mode.  */
	lfd	fp13,.LC0@toc(2)
	fabs	fp0,fp1
	fsub	fp12,fp13,fp13	/* generate 0.0  */
	fcmpu	cr7,fp0,fp13	/* if (fabs(x) > TWO52)  */
	fcmpu	cr6,fp1,fp12	/* if (x > 0.0)  */
	bnllr-	cr7
	mtfsfi	7,1		/* Set rounding mode toward 0.  */
	lfd	fp10,.LC1@toc(2)
	ble-	cr6,.L4
	fadd	fp1,fp1,fp10	/* x+= 0.5;  */
	fadd	fp1,fp1,fp13	/* x+= TWO52;  */
	fsub	fp1,fp1,fp13	/* x-= TWO52;  */
	fabs	fp1,fp1		/* if (x == 0.0)  */
				/* x = 0.0; */
	mtfsf	0x01,fp11	/* restore previous rounding mode.  */
	blr
.L4:
	fsub	fp9,fp1,fp10	/* x+= 0.5;  */
	bge-	cr6,.L9		/* if (x < 0.0)  */
	fsub	fp1,fp9,fp13	/* x-= TWO52;  */
	fadd	fp1,fp1,fp13	/* x+= TWO52;  */
	fnabs	fp1,fp1		/* if (x == 0.0)  */
				/* x = -0.0; */
.L9:
	mtfsf	0x01,fp11	/* restore previous rounding mode.  */	
	blr
	END (__round)

weak_alias (__round, round)

#ifdef NO_LONG_DOUBLE
weak_alias (__round, roundl)
strong_alias (__round, __roundl)
#endif
