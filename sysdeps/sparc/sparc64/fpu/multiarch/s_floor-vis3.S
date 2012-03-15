/* floor function, sparc64 vis3 version.
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

#define TWO_FIFTYTWO	0x43300000		/* 2**52 */
#define ONE_DOT_ZERO	0x3ff00000		/* 1.0 */

#define ZERO		%f10			/* 0.0 */
#define SIGN_BIT	%f12			/* -0.0 */

ENTRY (__floor_vis3)
	sethi	%hi(TWO_FIFTYTWO), %o2
	sethi	%hi(ONE_DOT_ZERO), %o3
	fzero	ZERO

	sllx	%o2, 32, %o2
	fnegd	ZERO, SIGN_BIT

	sllx	%o3, 32, %o3
	movxtod	%o2, %f16
	fabsd	%f0, %f14

	fcmpd	%fcc3, %f14, %f16

	fmovduge %fcc3, ZERO, %f16
	fand	%f0, SIGN_BIT, SIGN_BIT

	for	%f16, SIGN_BIT, %f16
	faddd	%f0, %f16, %f18
	fsubd	%f18, %f16, %f18
	fcmpd	%fcc2, %f18, %f0
	movxtod	%o3, %f20

	fmovdule %fcc2, ZERO, %f20
	fsubd	%f18, %f20, %f0
	fabsd	%f0, %f0
	retl
	 for	%f0, SIGN_BIT, %f0
END (__floor_vis3)
