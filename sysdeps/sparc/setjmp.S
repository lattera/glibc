/* Copyright (C) 1991, 1993, 1994 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <sysdep.h>

/* NOTE: This code depends on the definition of `__jmp_buf' in <jmp_buf.h>.  */

ENTRY (__sigsetjmp)
	/* Save our return PC and SP (second store in the jmp delay slot).  */
	st %o7, [%o0]
	/* Save the signal mask if requested.  We do this as a tail-call
	   for simplicity; it always returns zero.  */
	sethi %hi(C_SYMBOL_NAME (__sigjmp_save)), %g1
	or %lo(C_SYMBOL_NAME (__sigjmp_save)), %g1, %g1
	jmp %g1
	st %sp, [%o0 + 4]
