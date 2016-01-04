/* Round double to long int.  POWER6x PowerPC64 version.
   Copyright (C) 2006-2016 Free Software Foundation, Inc.
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

	.machine	"power6"
/* long long int[r3] __llrint (double x[fp1])  */
ENTRY (__llrint)
	CALL_MCOUNT 0
	fctid	fp13,fp1
	mftgpr  r3,fp13
	blr
	END (__llrint)

strong_alias (__llrint, __lrint)
weak_alias (__llrint, llrint)
weak_alias (__lrint, lrint)

#ifdef NO_LONG_DOUBLE
strong_alias (__llrint, __llrintl)
weak_alias (__llrint, llrintl)
strong_alias (__lrint, __lrintl)
weak_alias (__lrint, lrintl)
#endif
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __llrint, llrintl, GLIBC_2_1)
compat_symbol (libm, __lrint, lrintl, GLIBC_2_1)
#endif
