/* llround function.  POWER8 PowerPC64 version.
   Copyright (C) 2014-2018 Free Software Foundation, Inc.
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
#include <endian.h>
#include <math_ldbl_opt.h>
#include <libm-alias-float.h>
#include <libm-alias-double.h>

#define MFVSRD_R3_V1  .long 0x7c230066     /* mfvsrd  r3,vs1  */

/* long long [r3] llround (float x [fp1])  */

ENTRY_TOCLESS (__llround)
	CALL_MCOUNT 0
	frin	fp1,fp1	/* Round to nearest +-0.5.  */
	fctidz	fp1,fp1	/* Convert To Integer DW round toward 0.  */
	MFVSRD_R3_V1
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
