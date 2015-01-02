/* isnan().  PowerPC32 version.
   Copyright (C) 2008-2015 Free Software Foundation, Inc.
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

/* int __isnanf(x)  */
	.machine power5
EALIGN (__isnanf, 4, 0)
	stwu	r1,-32(r1)
	cfi_adjust_cfa_offset (32)
	stfs	fp1,28(r1)	/* copy FPR to GPR */
	nop
	nop
	lwz	r4,28(r1)
	lis	r0,0x7f80	/* const long r0 0x7f800000 */
	clrlwi	r4,r4,1		/* x = fabs(x) */
	cmpw	cr7,r4,r0	/* if (fabs(x) =< inf) */
	li	r3,0		/* then return 0 */
	addi	r1,r1,32
	cfi_adjust_cfa_offset (-32)
	blelr+	cr7
L(NaN):
	li	r3,1		/* else return 1 */
	blr
	END (__isnanf)

hidden_def (__isnanf)
weak_alias (__isnanf, isnanf)

