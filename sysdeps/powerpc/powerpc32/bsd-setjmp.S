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

#if defined SHARED && SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_3_4)


/* We need 2 copies of identical code for the  setjmp's as gas complains
   erroneously about having multiple versions of setjmp.  This eliminates the
   need for the strong alias of __setjmp to setjmp which
   does not support versioning, i.e. another gas unimplemented feature. */
symbol_version (__novmx__setjmp,__setjmp,GLIBC_2.0)
symbol_version (__novmxsetjmp,setjmp,GLIBC_2.0)

ENTRY (BP_SYM (__novmxsetjmp))
	li r4,1			/* Set second argument to 1.  */
	b JUMPTARGET (BP_SYM (__novmx__sigsetjmp))
END (BP_SYM (__novmxsetjmp))

ENTRY (BP_SYM (__novmx__setjmp))
	li r4,1			/* Set second argument to 1.  */
	b JUMPTARGET (BP_SYM (__novmx__sigsetjmp))
END (BP_SYM (__novmx__setjmp))
#endif  /* defined SHARED && SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_3_4) ) */

default_symbol_version (__vmx__setjmp,__setjmp,GLIBC_2.3.4)
default_symbol_version (__vmxsetjmp,setjmp,GLIBC_2.3.4)

ENTRY (BP_SYM (__vmxsetjmp))
	li r4,1			/* Set second argument to 1.  */
	b JUMPTARGET (BP_SYM (__vmx__sigsetjmp))
END (BP_SYM (__vmxsetjmp))

ENTRY (BP_SYM (__vmx__setjmp))
	li r4,1			/* Set second argument to 1.  */
	b JUMPTARGET (BP_SYM (__vmx__sigsetjmp))
END (BP_SYM (__vmx__setjmp))
