/* Compute positive difference, sparc 32-bit+v9.
   Copyright (C) 2013-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David S. Miller <davem@davemloft.net>.

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

ENTRY(__fdimf)
	st	%o0, [%sp + 72]
	st	%o1, [%sp + 76]
	ld	[%sp + 72], %f0
	ld	[%sp + 76], %f1
	fcmps	%f0, %f1
	fbug	1f
	 nop
	fzeros	%f0
	fnegs	%f0, %f1
1:	retl
	 fsubs	%f0, %f1, %f0
END(__fdimf)
weak_alias (__fdimf, fdimf)
