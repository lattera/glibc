/* Float ceil function, sparc32 v9 vis3 version.
   Copyright (C) 2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David S. Miller <davem@davemloft.net>, 2012.

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

	/* Since changing the rounding mode is extremely expensive, we
	   try to round up using a method that is rounding mode
	   agnostic.

	   We add then subtract (or subtract than add if the initial
	   value was negative) 2**23 to the value, then subtract it
	   back out.

	   This will clear out the fractional portion of the value.
	   One of two things will happen for non-whole initial values.
	   Either the rounding mode will round it up, or it will be
	   rounded down.  If the value started out whole, it will be
	   equal after the addition and subtraction.  This means we
	   can accurately detect with one test whether we need to add
	   another 1.0 to round it up properly.

	   VIS instructions are used to facilitate the formation of
	   easier constants, and the propagation of the sign bit.  */

#define TWO_TWENTYTHREE	0x4b000000		/* 2**23 */
#define ONE_DOT_ZERO	0x3f800000		/* 1.0 */

#define ZERO		%f10			/* 0.0 */
#define SIGN_BIT	%f12			/* -0.0 */

ENTRY (__ceilf_vis3)
	movwtos	%o0, %f0
	sethi	%hi(TWO_TWENTYTHREE), %o2
	sethi	%hi(ONE_DOT_ZERO), %o3
	fzeros	ZERO

	fnegs	ZERO, SIGN_BIT

	movwtos	%o2, %f16
	fabss	%f0, %f14

	fcmps	%fcc3, %f14, %f16

	fmovsuge %fcc3, ZERO, %f16
	fands	%f0, SIGN_BIT, SIGN_BIT

	fors	%f16, SIGN_BIT, %f16
	fadds	%f0, %f16, %f1
	fsubs	%f1, %f16, %f1
	fcmps	%fcc2, %f1, %f0
	movwtos	%o3, %f9

	fmovsuge %fcc2, ZERO, %f9
	fadds	%f1, %f9, %f0
	fabss	%f0, %f0
	retl
	 fors	%f0, SIGN_BIT, %f0
END (__ceilf_vis3)
