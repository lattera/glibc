/* Copy a sign bit between floating-point values.
   IBM extended format long double version.
   Copyright (C) 2006 Free Software Foundation, Inc.
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

ENTRY(__copysignl)
/* long double [f1,f2] copysign (long double [f1,f2] x, long double [f3,f4] y);
   copysign(x,y) returns a value with the magnitude of x and
   with the sign bit of y.  */
	stwu	r1,-16(r1)
	cfi_adjust_cfa_offset (16)
	stfd	fp3,8(r1)
	fmr	fp0,fp1
	fabs	fp1,fp1
	fcmpu	cr7,fp0,fp1
	lwz	r3,8(r1)
	cmpwi	cr6,r3,0
	addi	r1,r1,16
	cfi_adjust_cfa_offset (-16)
	beq	cr7,L(0)
	fneg	fp2,fp2
L(0):	bgelr	cr6
	fneg	fp1,fp1
	fneg	fp2,fp2
	blr
END (__copysignl)

#ifdef IS_IN_libm
long_double_symbol (libm, __copysignl, copysignl)
#else
long_double_symbol (libc, __copysignl, copysignl)
#endif
