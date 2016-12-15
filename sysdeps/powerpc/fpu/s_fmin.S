/* Floating-point minimum.  PowerPC version.
   Copyright (C) 1997-2016 Free Software Foundation, Inc.
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

ENTRY(__fmin)
/* double [f1] fmin (double [f1] x, double [f2] y); */
	fcmpu	cr0,fp1,fp2
	bgt	cr0,0f		/* if x > y, neither x nor y can be NaN... */
	bnulr+	cr0
/* x and y are unordered, so one of x or y must be a NaN... */
	fcmpu	cr1,fp2,fp2
	bun	cr1,1f
/* x is a NaN; y is not.  Test if x is signaling.  */
#ifdef __powerpc64__
	stfd	fp1,-8(r1)
	lwz	r3,-8+HIWORD(r1)
#else
	stwu	r1,-16(r1)
	cfi_adjust_cfa_offset (16)
	stfd	fp1,8(r1)
	lwz	r3,8+HIWORD(r1)
	addi	r1,r1,16
	cfi_adjust_cfa_offset (-16)
#endif
	andis.	r3,r3,8
	bne	cr0,0f
	b	2f
1:	/* y is a NaN; x may or may not be.  */
	fcmpu	cr1,fp1,fp1
	bun	cr1,2f
/* y is a NaN; x is not.  Test if y is signaling.  */
#ifdef __powerpc64__
	stfd	fp2,-8(r1)
	lwz	r3,-8+HIWORD(r1)
#else
	stwu	r1,-16(r1)
	cfi_adjust_cfa_offset (16)
	stfd	fp2,8(r1)
	lwz	r3,8+HIWORD(r1)
	addi	r1,r1,16
	cfi_adjust_cfa_offset (-16)
#endif
	andis.	r3,r3,8
	bnelr	cr0
2:	/* x and y are NaNs, or one is a signaling NaN.  */
	fadd	fp1,fp1,fp2
	blr
0:	fmr	fp1,fp2
	blr
END(__fmin)

weak_alias (__fmin,fmin)

/* It turns out that it's safe to use this code even for single-precision.  */
strong_alias(__fmin,__fminf)
weak_alias (__fmin,fminf)

#ifdef NO_LONG_DOUBLE
weak_alias (__fmin,__fminl)
weak_alias (__fmin,fminl)
#endif
