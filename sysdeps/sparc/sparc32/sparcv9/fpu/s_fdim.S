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
#include <math_ldbl_opt.h>

ENTRY(__fdim)
	std	%o0, [%sp + 72]
	std	%o2, [%sp + 80]
	ldd	[%sp + 72], %f0
	ldd	[%sp + 80], %f2
	fcmpd	%f0, %f2
	fbug	1f
	 nop
	fzero	%f0
	fnegd	%f0, %f2
1:	retl
	 fsubd	%f0, %f2, %f0
END(__fdim)
weak_alias (__fdim, fdim)

#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __fdim, fdiml, GLIBC_2_1);
#endif
