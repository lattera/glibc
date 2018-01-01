/* llround function.  POWER5+, PowerPC64 version.
   Copyright (C) 2006-2018 Free Software Foundation, Inc.
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
ENTRY_TOCLESS (__llround, 4)
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
libm_alias_double (__llround, llround)
libm_alias_double (__lround, lround)
/* The double version also works for single-precision as both float and
   double parameters are passed in 64bit FPRs and both versions are expected
   to return [long] long type.  */
strong_alias (__llround, __llroundf)
libm_alias_float (__llround, llround)
strong_alias (__lround, __lroundf)
libm_alias_float (__lround, lround)
