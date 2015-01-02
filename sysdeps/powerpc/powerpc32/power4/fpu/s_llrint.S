/* Round double to long int.  PowerPC32 on PowerPC64 version.
   Copyright (C) 2004-2015 Free Software Foundation, Inc.
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

/* long long int[r3, r4] __llrint (double x[fp1])  */
ENTRY (__llrint)
	CALL_MCOUNT
	stwu	r1,-16(r1)
	cfi_adjust_cfa_offset (16)
	fctid	fp13,fp1
	stfd	fp13,8(r1)
	nop	/* Insure the following load is in a different dispatch group */
	nop	/* to avoid pipe stall on POWER4&5.  */
	nop
	lwz	r3,8+HIWORD(r1)
	lwz	r4,8+LOWORD(r1)
	addi	r1,r1,16
	blr
	END (__llrint)

weak_alias (__llrint, llrint)

#ifdef NO_LONG_DOUBLE
strong_alias (__llrint, __llrintl)
weak_alias (__llrint, llrintl)
#endif
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __llrint, llrintl, GLIBC_2_1)
#endif
