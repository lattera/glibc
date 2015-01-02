/* Search a character in a block of memory.  For IBM S390
   Copyright (C) 2000-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
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
 * R3 = character to find
 * R4 = number of bytes to search
 */

#include "sysdep.h"
#include "asm-syntax.h"

	.text
ENTRY(memchr)
	lhi   %r0,0xff
	nr    %r0,%r3
	lr    %r1,%r2
	la    %r2,0(%r4,%r1)
0:      srst  %r2,%r1
	jo    0b
	brc   13,1f
	slr   %r2,%r2
1:      br    %r14
END(memchr)
libc_hidden_builtin_def (memchr)
