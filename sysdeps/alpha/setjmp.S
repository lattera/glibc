/* Copyright (C) 1992 Free Software Foundation, Inc.
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

/* The function __setjmp_aux saves all the registers, but it can't
   reliably access the stack or frame pointers, so we pass them in as
   extra arguments.  */
ENTRY (__setjmp)
	lda $27, __setjmp_aux	/* Load address to jump to.  */
	bis $15, $15, $17	/* Pass FP as 2nd arg.  */
	bis $30, $30, $18	/* Pass SP as 3nd arg.  */
	jmp $31, ($27), __setjmp_aux /* Call __setjmp_aux.  */
	.end __setjmp
