/* Copyright (C) 2000-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <shlib-compat.h>

	.arch ev6
	.set noreorder
	.set noat

ENTRY(__ieee754_sqrtf)
#ifdef PROF
	ldgp    gp, 0(pv)
	lda     AT, _mcount
	jsr     AT, (AT), _mcount
	.prologue 1
#else
	.prologue 0
#endif

	.align 4
#ifdef _IEEE_FP_INEXACT
	sqrts/suid $f16, $f0
#else
	sqrts/sud $f16, $f0
#endif
	ret
	nop
	nop

END(__ieee754_sqrtf)

#if SHLIB_COMPAT (libm, GLIBC_2_15, GLIBC_2_18)
strong_alias(__ieee754_sqrtf, __sqrtf_finite1)
compat_symbol(libm, __sqrtf_finite1, __sqrtf_finite, GLIBC_2_15)
versioned_symbol(libm, __ieee754_sqrtf, __sqrtf_finite, GLIBC_2_18)
#else
strong_alias(__ieee754_sqrtf, __sqrtf_finite)
#endif
