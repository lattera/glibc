/* BSD `setjmp' entry point to `sigsetjmp (..., 1)'.  PowerPC32/64 version.
   Copyright (C) 1994,1997,1999,2000,2003,2004 Free Software Foundation, Inc.
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

#if defined SHARED && SHLIB_COMPAT (libc, GLIBC_2_3, GLIBC_2_3_4)


ENTRY (__novmxsetjmp)
	CALL_MCOUNT 1
	li r4,1			/* Set second argument to 1.  */
	b JUMPTARGET (__novmx__sigsetjmp_ent)
END (__novmxsetjmp)
strong_alias (__novmxsetjmp, __novmx__setjmp)
symbol_version (__novmxsetjmp, setjmp, GLIBC_2.3)

#endif  /*  defined SHARED && SHLIB_COMPAT (libc, GLIBC_2_3, GLIBC_2_3_4) */


ENTRY (__vmxsetjmp)
	CALL_MCOUNT 1
	li r4,1			/* Set second argument to 1.  */
	b JUMPTARGET (__vmx__sigsetjmp_ent)
END (__vmxsetjmp)
strong_alias (__vmxsetjmp, __vmx__setjmp)
strong_alias (__vmx__sigsetjmp, __setjmp)
default_symbol_version (__vmxsetjmp, setjmp, GLIBC_2.3.4)
