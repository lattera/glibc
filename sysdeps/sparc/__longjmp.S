/* Copyright (C) 1991, 1993 Free Software Foundation, Inc.
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
#ifdef __svr4__
#include <sys/trap.h>
#else
#include <machine/trap.h>
#endif

/* NOTE: This code depends on the definition of `__jmp_buf' in <jmp_buf.h>.  */

ENTRY (__longjmp)
	/* Do a "flush register windows trap".  The trap handler in the
	   kernel writes all the register windows to their stack slots, and
	   marks them all as invalid (needing to be sucked up from the
	   stack when used).  This ensures that all information needed to
	   unwind to these callers is in memory, not in the register
	   windows.  */
	ta ST_FLUSH_WINDOWS
	ld [%o0], %o7		/* Return PC.  */
	ld [%o0 + 4], %fp	/* Saved SP.  */
	sub %fp, 64, %sp	/* Allocate a register save area.  */

	/* if (%o1 == 0) %o1 = 1; */
	tst %o1
	be,a Ldone
	mov 1, %o1

Ldone:	retl
	/* On the way out, put the return value in %o0.  */
	restore %o1, 0, %o0
