/* sqrtf function.  sparc32 v9 version.
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

ENTRY (__sqrtf)
	st	%o0, [%sp + 72]
	fzeros	%f8
	ld	[%sp + 72], %f0
	fcmps	%f0, %f8
	fbl	1f
	 nop
8:	retl
	 fsqrts	%f0, %f0
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
	 mov	%o0, %o1
	mov	126, %o2
	mov	%o7, %g1
	call	__kernel_standard_f
	 mov	%g1, %o7
END (__sqrtf)

weak_alias (__sqrtf, sqrtf)
