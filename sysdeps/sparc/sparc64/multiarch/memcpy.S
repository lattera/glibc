/* Multiple versions of memcpy
   Copyright (C) 2010, 2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include <rtld-global-offsets.h>

#if !defined NOT_IN_libc
	.text
ENTRY(memcpy)
	.type	memcpy, @gnu_indirect_function
# ifdef SHARED
	mov	%o7, %o5
	sethi	%hi(_GLOBAL_OFFSET_TABLE_-4), %o3
	call	1f
	 or	%o3, %lo(_GLOBAL_OFFSET_TABLE_+4), %o3
1:	add	%o7, %o3, %o3
	mov	%o5, %o7
# endif
	andcc	%o0, HWCAP_SPARC_N2, %g0
	be	1f
	 andcc	%o0, HWCAP_SPARC_BLKINIT, %g0
# ifdef SHARED
	sethi	%gdop_hix22(__memcpy_niagara2), %o1
	xor	%o1, %gdop_lox10(__memcpy_niagara2), %o1
# else
	set	__memcpy_niagara2, %o1
# endif
	ba	10f
	 nop
1:	be	1f
	 andcc	%o0, HWCAP_SPARC_ULTRA3, %g0
# ifdef SHARED
	sethi	%gdop_hix22(__memcpy_niagara1), %o1
	xor	%o1, %gdop_lox10(__memcpy_niagara1), %o1
# else
	set	__memcpy_niagara1, %o1
# endif
	ba	10f
	 nop
1:	be	9f
	 nop
# ifdef SHARED
	sethi	%gdop_hix22(__memcpy_ultra3), %o1
	xor	%o1, %gdop_lox10(__memcpy_ultra3), %o1
# else
	set	__memcpy_ultra3, %o1
# endif
	ba	10f
	 nop
9:
# ifdef SHARED
	sethi	%gdop_hix22(__memcpy_ultra1), %o1
	xor	%o1, %gdop_lox10(__memcpy_ultra1), %o1
# else
	set	__memcpy_ultra1, %o1
# endif
10:
# ifdef SHARED
	add	%o3, %o1, %o1
# endif
	retl
	 mov	%o1, %o0
END(memcpy)

# undef libc_hidden_builtin_def
/* IFUNC doesn't work with the hidden functions in a shared library.  */
# define libc_hidden_builtin_def(name) \
	.globl __GI_memcpy; __GI_memcpy = __memcpy_ultra1

#define memcpy __memcpy_ultra1

#endif

#include "../memcpy.S"
