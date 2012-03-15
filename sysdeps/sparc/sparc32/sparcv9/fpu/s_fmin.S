/* fmin function, sparc32 v9 version.
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

#define SIGN_BIT	%f12			/* -0.0 */

ENTRY (__fmin)
	std	%o0, [%sp + 72]
	std	%o2, [%sp + 80]
	ldd	[%sp + 72], %f0
	ldd	[%sp + 80], %f2
	fcmpd	%fcc1, %f2, %f2
	fzero	SIGN_BIT
	fmovdu	%fcc1, %f0, %f2
	fnegd	SIGN_BIT, SIGN_BIT
	fcmpd	%fcc2, %f0, %f2
	fmovdug	%fcc2, %f2, %f0
	for	%f0, %f2, %f4
	fand	%f4, SIGN_BIT, %f4
	retl
	 for	%f0, %f4, %f0
END (__fmin)
weak_alias (__fmin, fmin)
