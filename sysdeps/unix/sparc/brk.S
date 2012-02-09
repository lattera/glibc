/* Copyright (C) 1993, 1995, 1997, 2005 Free Software Foundation, Inc.
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

#ifndef	SYS_brk
#define	SYS_brk	17
#endif

#ifndef C_SYMBOL_NAME
#define	C_SYMBOL_NAME(name)	_##name
#endif

.data
.global C_SYMBOL_NAME(__curbrk)
C_LABEL(__curbrk)
	.long C_SYMBOL_NAME(_end)

.text
ENTRY (__brk)
	add %o0, 7, %o0
	andn %o0, 7, %o0
	mov SYS_brk, %g1
	mov %o0, %o1		/* Save rounded value.  */
	ta %g0
	bcs error
	sethi %hi(C_SYMBOL_NAME(__curbrk)), %g1
	st %o1, [%g1 + %lo(C_SYMBOL_NAME(__curbrk))]
	ret
error:	sethi %hi(C_SYMBOL_NAME(errno)), %g1
	st %o0, [%g1 + %lo(C_SYMBOL_NAME(errno))]
	sub %g0, 1, %o0
	retl
	nop			/* Fill the delay slot.  */

weak_alias (__brk, brk)
