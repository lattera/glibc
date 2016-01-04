/* llround function.  POWER8 PowerPC64 version.
   Copyright (C) 2014-2016 Free Software Foundation, Inc.
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

#define MFVSRD_R3_V1  .long 0x7c230066     /* mfvsrd  r3,vs1  */

/* long long [r3] llround (float x [fp1])  */

ENTRY (__llround)
	CALL_MCOUNT 0
	frin	fp1,fp1	/* Round to nearest +-0.5.  */
	fctidz	fp1,fp1	/* Convert To Integer DW round toward 0.  */
	MFVSRD_R3_V1
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
