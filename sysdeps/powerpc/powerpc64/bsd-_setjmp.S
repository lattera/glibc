/* BSD `_setjmp' entry point to `sigsetjmp (..., 0)'.  PowerPC32/64 version.
   Copyright (C) 1994, 1997, 1999, 2000, 2002, 2003, 2004
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */
#include <shlib-compat.h>
#include <libc-symbols.h>
#include <sysdep.h>
#include <bp-sym.h>

#if defined NOT_IN_libc
/* Build a non-versioned object for rtld-*.  */
ENTRY (BP_SYM (_setjmp))
	CALL_MCOUNT 1
	li r4,0			/* Set second argument to 0.  */
	b JUMPTARGET (__sigsetjmp_ent)
END (BP_SYM (_setjmp))
libc_hidden_def (_setjmp)

#else
/* Build a versioned object for libc.  */
# if defined SHARED && SHLIB_COMPAT (libc, GLIBC_2_3, GLIBC_2_3_4)
symbol_version (__novmx_setjmp,_setjmp,GLIBC_2.3);

ENTRY (BP_SYM (__novmx_setjmp))
	CALL_MCOUNT 1
	li r4,0			/* Set second argument to 0.  */
	b JUMPTARGET (__novmx__sigsetjmp_ent)
END (BP_SYM (__novmx_setjmp))
libc_hidden_def (__novmx_setjmp)
# endif /* defined SHARED && SHLIB_COMPAT (libc, GLIBC_2_3, GLIBC_2_3_4) */

default_symbol_version (__vmx_setjmp,_setjmp,GLIBC_2.3.4)
/* __GI__setjmp prototype is needed for ntpl i.e. _setjmp is defined
   as a libc_hidden_proto & is used in sysdeps/generic/libc-start.c
   if HAVE_CLEANUP_JMP_BUF is defined */
ENTRY (BP_SYM (__GI__setjmp))
#if defined SHARED && !defined IS_IN_rtld
	std r2,40(r1)	/* Save the callers TOC in the save area.  */
#endif
	CALL_MCOUNT 1
	li r4,0			/* Set second argument to 0.  */
	b JUMPTARGET (__vmx__sigsetjmp_ent)
END (BP_SYM (__GI__setjmp))

ENTRY (BP_SYM (__vmx_setjmp))
	CALL_MCOUNT 1
	li r4,0			/* Set second argument to 0.  */
	b JUMPTARGET (__vmx__sigsetjmp_ent)
END (BP_SYM (__vmx_setjmp))
libc_hidden_def (__vmx_setjmp)
#endif /* !NOT_IN_libc */
