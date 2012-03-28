/* Optimized bzero `implementation' for PowerPC.
   Copyright (C) 1997, 1999, 2000 Free Software Foundation, Inc.
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
#include <bp-sym.h>

ENTRY (BP_SYM (__bzero))

#if __BOUNDED_POINTERS__
	mr	r6,r4
	li	r5,0
	mr	r4,r3
	/* Tell memset that we don't want a return value.  */
	li	r3,0
#else
	mr	r5,r4
	li	r4,0
#endif
	b	BP_SYM (memset)@local
END (BP_SYM (__bzero))
weak_alias (BP_SYM (__bzero), BP_SYM (bzero))
