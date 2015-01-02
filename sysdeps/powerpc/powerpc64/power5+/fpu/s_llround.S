/* llround function.  POWER5+, PowerPC64 version.
   Copyright (C) 2006-2015 Free Software Foundation, Inc.
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

/* long long [r3] llround (float x [fp1])
   IEEE 1003.1 llround function.  IEEE specifies "round to the nearest
   integer value, rounding halfway cases away from zero, regardless of
   the current rounding mode."  However PowerPC Architecture defines
   "round to Nearest" as "Choose the best approximation. In case of a
   tie, choose the one that is even (least significant bit o).".
   So we pre-round using the V2.02 Floating Round to Integer Nearest
   instruction before we use Floating Convert to Integer Word with
   round to zero instruction.  */

	.machine	"power5"
EALIGN (__llround, 4, 0)
	CALL_MCOUNT 0
	frin	fp2, fp1	/* Round to nearest +-0.5.  */
	fctidz	fp3, fp2	/* Convert To Integer DW round toward 0.  */
	stfd	fp3, -16(r1)
	nop	/* Insure the following load is in a different dispatch group */
	nop	/* to avoid pipe stall on POWER4&5.  */
	nop
	ld	r3, -16(r1)
	blr
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
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __llround, llroundl, GLIBC_2_1)
compat_symbol (libm, __lround, lroundl, GLIBC_2_1)
#endif
