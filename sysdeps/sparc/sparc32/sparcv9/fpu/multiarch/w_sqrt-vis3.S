/* sqrt function.  sparc32 v9 vis3 version.
   Copyright (C) 2012 Free Software Foundation, Inc.
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

ENTRY (__sqrt_vis3)
	movwtos	%o0, %f0
	fzero	%f8
	movwtos	%o1, %f1
	fcmpd	%f0, %f8
	fbl	1f
	 nop
8:	retl
	 fsqrtd	%f0, %f0
1:
#ifdef SHARED
	SETUP_PIC_REG_LEAF(o5, g1)
	sethi	%gdop_hix22(_LIB_VERSION), %g1
	xor	%g1, %gdop_lox10(_LIB_VERSION), %g1
	ld	[%o5 + %g1], %g1, %gdop(_LIB_VERSION)
#else
	sethi	%hi(_LIB_VERSION), %g1
	or	%g1, %lo(_LIB_VERSION), %g1
#endif
	ld	[%g1], %g1
	cmp	%g1, -1
	be	8b
	 mov	%o0, %o2
	mov	%o1, %o3
	mov	26, %o4
	mov	%o7, %g1
	call	__kernel_standard
	 mov	%g1, %o7
END (__sqrt_vis3)
