/* lround function.  POWER5+, PowerPC32 version.
   Copyright (C) 2006, 2007 Free Software Foundation, Inc.
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
   Software Foundation, Inc., 1 Franklin Street, Fifth Floor, Boston MA
   02110-1301 USA.  */

#include <sysdep.h>
#include <math_ldbl_opt.h>
	
/* long [r3] llround (float x [fp1])
   IEEE 1003.1 lround function.  IEEE specifies "round to the nearest 
   integer value, rounding halfway cases away from zero, regardless of
   the current rounding mode."  However PowerPC Architecture defines
   "round to Nearest" as "Choose the best approximation. In case of a 
   tie, choose the one that is even (least significant bit o).". 
   So we pre-round using the V2.02 Floating Round to Integer Nearest
   instruction before we use the Floating Convert to Integer Word with
   round to zero instruction.  */

	.machine	"power5"
ENTRY (__llround)
	stwu    r1,-16(r1)
	cfi_adjust_cfa_offset (16)
	frin	fp2,fp1
	fctidz	fp3,fp2		/* Convert To Integer Word lround toward 0.  */
	stfd	fp3,8(r1)
/* Insure the following load is in a different dispatch group by
   inserting "group ending nop".  */
	ori	r1,r1,0
	lwz	r4,12(r1)
	lwz	r3,8(r1)
	addi	r1,r1,16
	blr
	END (__llround)

weak_alias (__llround, llround)

strong_alias (__llround, __llroundf)
weak_alias (__llround, llroundf)

#ifdef NO_LONG_DOUBLE
weak_alias (__llround, llroundl)
strong_alias (__llround, __llroundl)
#endif
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __llround, llroundl, GLIBC_2_1)
#endif
