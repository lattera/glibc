/* trunc function.  PowerPC64/power5+ version.
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

	.machine	"power5"
EALIGN (__trunc, 4, 0)
	CALL_MCOUNT 0
	friz	fp1, fp1
	blr
	END (__trunc)

weak_alias (__trunc, trunc)

#ifdef NO_LONG_DOUBLE
weak_alias (__trunc, truncl)
strong_alias (__trunc, __truncl)
#endif
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __trunc, truncl, GLIBC_2_1)
#endif
