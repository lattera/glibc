/* BSD `_setjmp' entry point to `sigsetjmp (..., 0)'.  Sparc version.
   Copyright (C) 1994, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sysdep.h>

ENTRY (_setjmp)
#ifdef PIC
1:
	jmpl  2f,%o1
	nop
2:
	sethi %hi(_GLOBAL_OFFSET_TABLE_-(1b-.)),%l7
	or    %l7,%lo(_GLOBAL_OFFSET_TABLE_-(1b-.)),%l7
	add   %l7,%o1,%l7
	sethi %hi(C_SYMBOL_NAME (__sigsetjmp)),%g1
	or    %g1,%lo(C_SYMBOL_NAME (__sigsetjmp)),%g1
	ld    [%l7+%g1],%g1
	ld    [%g1],%g1
	jmpl  %g1,%g0
	mov   %g0,%o1           /* Pass second argument of zero */
#else

	sethi %hi(C_SYMBOL_NAME (__sigsetjmp)), %g1
	or %lo(C_SYMBOL_NAME (__sigsetjmp)), %g1, %g1
	jmp %g1
	mov %g0, %o1		/* Pass second argument of zero.  */
#endif
