/* Round double to long int.  PowerPC64 version.
   Copyright (C) 2004-2018 Free Software Foundation, Inc.
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
#include <libm-alias-float.h>
#include <libm-alias-double.h>

/* long long int[r3] __llrint (double x[fp1])  */
ENTRY_TOCLESS (__llrint)
	CALL_MCOUNT 0
	fctid	fp13,fp1
	stfd	fp13,-16(r1)
	nop	/* Insure the following load is in a different dispatch group */
	nop	/* to avoid pipe stall on POWER4&5.  */
	nop
	ld	r3,-16(r1)
	blr
	END (__llrint)

strong_alias (__llrint, __lrint)
libm_alias_double (__llrint, llrint)
libm_alias_double (__lrint, lrint)
/* The double version also works for single-precision as both float and
   double parameters are passed in 64bit FPRs and both versions are expected
   to return [long] long type.  */
strong_alias (__llrint, __llrintf)
libm_alias_float (__llrint, llrint)
strong_alias (__lrint, __lrintf)
libm_alias_float (__lrint, lrint)
