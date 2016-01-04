/* bzero -- set a block of memory to zero.  IBM S390 version
   This file is part of the GNU C Library.
   Copyright (C) 2000-2016 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).

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

/*
 * R2 = address to memory area
 * R3 = number of bytes to fill
 */

#include "sysdep.h"
#include "asm-syntax.h"

	.text
ENTRY(__bzero)
	ltr     %r3,%r3
	jz      .L1
	sr      %r1,%r1             # set pad byte to zero
	sr      %r4,%r4             # no source for MVCLE, only a pad byte
	sr      %r5,%r5
.L0:    mvcle   %r2,%r4,0(%r1)      # thats it, MVCLE is your friend
	jo      .L0
.L1:    br      %r14
END(__bzero)

#ifndef NO_WEAK_ALIAS
weak_alias (__bzero, bzero)
#endif
