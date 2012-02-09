/* BSD `_setjmp' entry point to `sigsetjmp (..., 0)'.  PowerPC32/64 version.
   Copyright (C) 1994,1997,1999,2000,2002,2003,2004
   Free Software Foundation, Inc.
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
#include <shlib-compat.h>
#include <libc-symbols.h>
#include <sysdep.h>
#include <bp-sym.h>

#if defined NOT_IN_libc
/* Build a non-versioned object for rtld-*.  */
ENTRY (BP_SYM (_setjmp))
	li r4,0			/* Set second argument to 0.  */
	b BP_SYM (__sigsetjmp@local)
END (BP_SYM (_setjmp))
libc_hidden_def (_setjmp)
#else
/* Build a versioned object for libc.  */

# if defined SHARED && SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_3_4)
symbol_version (__novmx_setjmp,_setjmp,GLIBC_2.0);

ENTRY (BP_SYM (__novmx_setjmp))
	li r4,0			/* Set second argument to 0.  */
	b BP_SYM (__novmx__sigsetjmp@local)
END (BP_SYM (__novmx_setjmp))
libc_hidden_def (__novmx_setjmp)
# endif /* defined SHARED  && SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_3_4) */

default_symbol_version (__vmx_setjmp,_setjmp,GLIBC_2.3.4)
/* __GI__setjmp prototype is needed for ntpl i.e. _setjmp is defined
   as a libc_hidden_proto & is used in sysdeps/generic/libc-start.c
   if HAVE_CLEANUP_JMP_BUF is defined */
ENTRY (BP_SYM (__GI__setjmp))
	li r4,0			/* Set second argument to 0.  */
	b BP_SYM (__vmx__sigsetjmp@local)
END (BP_SYM (__GI__setjmp))

ENTRY (BP_SYM (__vmx_setjmp))
	li r4,0			/* Set second argument to 0.  */
	b BP_SYM (__vmx__sigsetjmp@local)
END (BP_SYM (__vmx_setjmp))
libc_hidden_def (__vmx_setjmp)
#endif /* !NOT_IN_libc */
