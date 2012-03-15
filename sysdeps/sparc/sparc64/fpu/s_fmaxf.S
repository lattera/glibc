/* fmaxf function, sparc64 version.
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

ENTRY (__fmaxf)
	fcmps	%fcc1, %f3, %f3
	fzeros	SIGN_BIT
	fmovsu	%fcc1, %f1, %f3
	fnegs	SIGN_BIT, SIGN_BIT
	fcmps	%fcc2, %f1, %f3
	fmovsul	%fcc2, %f3, %f1
	fands	%f1, %f3, %f4
	fandnot2s SIGN_BIT, %f4, %f4
	retl
	 fandnot2s %f1, %f4, %f0
END (__fmaxf)
weak_alias (__fmaxf, fmaxf)
