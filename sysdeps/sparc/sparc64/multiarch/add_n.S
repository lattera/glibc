/* Multiple versions of add_n

   Copyright (C) 2013-2016 Free Software Foundation, Inc.
   Contributed by David S. Miller (davem@davemloft.net)
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

ENTRY(__mpn_add_n)
	.type	__mpn_add_n, @gnu_indirect_function
# ifdef SHARED
	SETUP_PIC_REG_LEAF(o3, o5)
# endif
	set	HWCAP_SPARC_VIS3, %o1
	andcc	%o0, %o1, %g0
	be	1f
	 nop
# ifdef SHARED
	sethi	%gdop_hix22(__mpn_add_n_vis3), %o1
	xor	%o1, %gdop_lox10(__mpn_add_n_vis3), %o1
# else
	set	__mpn_add_n_vis3, %o1
# endif
	ba	10f
	 nop
1:
# ifdef SHARED
	sethi	%gdop_hix22(__mpn_add_n_generic), %o1
	xor	%o1, %gdop_lox10(__mpn_add_n_generic), %o1
# else
	set	__mpn_add_n_generic, %o1
# endif
10:
# ifdef SHARED
	add	%o3, %o1, %o1
# endif
	retl
	 mov	%o1, %o0
END(__mpn_add_n)

#define __mpn_add_n __mpn_add_n_generic
#include "../add_n.S"
