/* Copyright (C) 2007-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <math_ldbl_opt.h>

        .set noat
	.set noreorder

	.text
ENTRY (__fmin)
	.prologue 0

	cmptun/su	$f16, $f16, $f10
	cmptun/su	$f17, $f17, $f11
	fmov		$f17, $f0
	unop

	trapb
	fbne		$f10, $ret
	fmov		$f16, $f0
	fbne		$f11, $ret

	cmptlt/su	$f17, $f16, $f11
	trapb
	fcmovne		$f11, $f17, $f0
$ret:	ret

END (__fmin)

/* Given the in-register format of single-precision, this works there too.  */
strong_alias (__fmin, __fminf)
weak_alias (__fminf, fminf)

weak_alias (__fmin, fmin)
#ifdef NO_LONG_DOUBLE
strong_alias (__fmin, __fminl)
weak_alias (__fminl, fminl)
#endif
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __fmin, fminl, GLIBC_2_1);
#endif
