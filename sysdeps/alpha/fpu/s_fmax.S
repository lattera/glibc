/* Copyright (C) 2007 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include <math_ldbl_opt.h>

        .set noat
	.set noreorder

	.text
ENTRY (__fmax)
	.prologue 0

	cmptun/su	$f16, $f16, $f10
	cmptun/su	$f17, $f17, $f11
	fmov		$f17, $f0
	unop

	trapb
	fbne		$f10, $ret
	fmov		$f16, $f0
	fbne		$f11, $ret

	cmptlt/su	$f16, $f17, $f11
	trapb
	fcmovne		$f11, $f17, $f0
$ret:	ret

END (__fmax)

/* Given the in-register format of single-precision, this works there too.  */
strong_alias (__fmax, __fmaxf)
weak_alias (__fmaxf, fmaxf)

weak_alias (__fmax, fmax)
#ifdef NO_LONG_DOUBLE
strong_alias (__fmax, __fmaxl)
weak_alias (__fmaxl, fmaxl)
#endif
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __fmax, fmaxl, GLIBC_2_1);
#endif
