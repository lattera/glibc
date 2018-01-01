/* Round double to long int.  PowerPC32 version.
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

/* long int[r3] __lrint (double x[fp1])  */
ENTRY (__lrint)
	stwu	r1,-16(r1)
	fctiw	fp13,fp1
	stfd	fp13,8(r1)
	nop	/* Ensure the following load is in a different dispatch group */
	nop	/* to avoid pipe stall on POWER4&5.  */
	nop
	lwz	r3,8+LOWORD(r1)
	addi	r1,r1,16
	blr
	END (__lrint)

libm_alias_double (__lrint, lrint)

strong_alias (__lrint, __lrintf)
libm_alias_float (__lrint, lrint)
