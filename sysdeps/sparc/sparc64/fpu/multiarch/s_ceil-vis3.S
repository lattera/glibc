/* ceil function, sparc64 vis3 version.
   Copyright (C) 2012-2015 Free Software Foundation, Inc.
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

	/* 'siam' (Set Interval Arithmetic Mode) is used to quickly override
	   the rounding mode during this routine.

	   We add then subtract (or subtract than add if the initial
	   value was negative) 2**23 to the value, then subtract it
	   back out.

	   This will clear out the fractional portion of the value and,
	   with suitable 'siam' initiated rouding mode settings, round
	   the final result in the proper direction.

	   We also use VIS3 moves to avoid using the stack to transfer
	   values between float and integer registers.  */

#define TWO_FIFTYTWO	0x43300000		/* 2**52 */

#define ZERO		%f10			/* 0.0 */
#define SIGN_BIT	%f12			/* -0.0 */

ENTRY (__ceil_vis3)
	sethi	%hi(TWO_FIFTYTWO), %o2
	fzero	ZERO
	sllx	%o2, 32, %o2
	fnegd	ZERO, SIGN_BIT
	movxtod	%o2, %f16
	fabsd	%f0, %f14
	fcmpd	%fcc3, %f14, %f16
	fmovduge %fcc3, ZERO, %f16
	fand	%f0, SIGN_BIT, SIGN_BIT
	for	%f16, SIGN_BIT, %f16
	siam	(1 << 2) | 2
	faddd	%f0, %f16, %f18
	siam	(1 << 2) | 0
	fsubd	%f18, %f16, %f18
	siam	(0 << 2)
	retl
	 for	%f18, SIGN_BIT, %f0
END (__ceil_vis3)
