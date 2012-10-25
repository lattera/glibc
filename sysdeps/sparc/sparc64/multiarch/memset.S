/* Multiple versions of memset and bzero
   All versions must be listed in ifunc-impl-list.c.
   Copyright (C) 2010, 2011, 2012 Free Software Foundation, Inc.
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

#if !defined NOT_IN_libc
	.text
ENTRY(memset)
	.type	memset, @gnu_indirect_function
# ifdef SHARED
	SETUP_PIC_REG_LEAF(o3, o5)
# endif
	set	HWCAP_SPARC_CRYPTO, %o1
	andcc	%o0, %o1, %g0
	be	1f
	 andcc	%o0, HWCAP_SPARC_BLKINIT, %g0
# ifdef SHARED
	sethi	%gdop_hix22(__memset_niagara4), %o1
	xor	%o1, %gdop_lox10(__memset_niagara4), %o1
# else
	set	__memset_niagara4, %o1
# endif
	ba	10f
	 nop
1:	be	9f
	 nop
# ifdef SHARED
	sethi	%gdop_hix22(__memset_niagara1), %o1
	xor	%o1, %gdop_lox10(__memset_niagara1), %o1
# else
	set	__memset_niagara1, %o1
# endif
	ba	10f
	 nop
9:
# ifdef SHARED
	sethi	%gdop_hix22(__memset_ultra1), %o1
	xor	%o1, %gdop_lox10(__memset_ultra1), %o1
# else
	set	__memset_ultra1, %o1
# endif
10:
# ifdef SHARED
	add	%o3, %o1, %o1
# endif
	retl
	 mov	%o1, %o0
END(memset)

ENTRY(__bzero)
	.type	bzero, @gnu_indirect_function
# ifdef SHARED
	SETUP_PIC_REG_LEAF(o3, o5)
# endif
	set	HWCAP_SPARC_CRYPTO, %o1
	andcc	%o0, %o1, %g0
	be	1f
	 andcc	%o0, HWCAP_SPARC_BLKINIT, %g0
# ifdef SHARED
	sethi	%gdop_hix22(__bzero_niagara4), %o1
	xor	%o1, %gdop_lox10(__bzero_niagara4), %o1
# else
	set	__bzero_niagara4, %o1
# endif
	ba	10f
	 nop
1:	be	9f
	 nop
# ifdef SHARED
	sethi	%gdop_hix22(__bzero_niagara1), %o1
	xor	%o1, %gdop_lox10(__bzero_niagara1), %o1
# else
	set	__bzero_niagara1, %o1
# endif
	ba	10f
	 nop
9:
# ifdef SHARED
	sethi	%gdop_hix22(__bzero_ultra1), %o1
	xor	%o1, %gdop_lox10(__bzero_ultra1), %o1
# else
	set	__bzero_ultra1, %o1
# endif
10:
# ifdef SHARED
	add	%o3, %o1, %o1
# endif
	retl
	 mov	%o1, %o0
END(__bzero)

weak_alias (__bzero, bzero)

# undef weak_alias
# define weak_alias(a, b)

libc_hidden_builtin_def (memset)

#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name)

#define memset __memset_ultra1
#define __bzero __bzero_ultra1

#endif

#include "../memset.S"
