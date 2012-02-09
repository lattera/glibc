/* Copy a sign bit between floating-point values.
   Copyright (C) 1997, 1999, 2000, 2006 Free Software Foundation, Inc.
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

/* This has been coded in assembler because GCC makes such a mess of it
   when it's coded in C.  */

#include <sysdep.h>
#include <math_ldbl_opt.h>

ENTRY(__copysign)
/* double [f1] copysign (double [f1] x, double [f2] y);
   copysign(x,y) returns a value with the magnitude of x and
   with the sign bit of y.  */
	stwu	r1,-16(r1)
	cfi_adjust_cfa_offset (16)
	stfd	fp2,8(r1)
	lwz	r3,8(r1)
	cmpwi   r3,0
	addi    r1,r1,16
	cfi_adjust_cfa_offset (-16)
	blt     L(0)
	fabs    fp1,fp1
	blr
L(0):	fnabs   fp1,fp1
	blr
	END (__copysign)

weak_alias (__copysign,copysign)

/* It turns out that it's safe to use this code even for single-precision.  */
weak_alias (__copysign,copysignf)
strong_alias(__copysign,__copysignf)

#ifdef NO_LONG_DOUBLE
weak_alias (__copysign,copysignl)
strong_alias(__copysign,__copysignl)
#endif
#ifdef IS_IN_libm
# if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __copysign, copysignl, GLIBC_2_0)
# endif
#elif LONG_DOUBLE_COMPAT(libc, GLIBC_2_0)
compat_symbol (libc, __copysign, copysignl, GLIBC_2_0)
#endif
