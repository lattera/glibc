/* Copyright (C) 2000 Free Software Foundation, Inc.
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
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>

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
	sqrts/sui $f16, $f0
#else
	sqrts/su $f16, $f0
#endif
	ret
	nop
	nop

END(__ieee754_sqrtf)
