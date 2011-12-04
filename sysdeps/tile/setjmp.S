/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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

#include <sysdep.h>
#include <jmpbuf-offsets.h>

	.text

	/* Keep traditional entry points in with sigsetjmp(). */
ENTRY(setjmp)
	{ movei r1, 1; j 1f }
END(setjmp)

ENTRY(_setjmp)
	{ movei r1, 0; j 1f }
END(_setjmp)
libc_hidden_def (_setjmp)

ENTRY(__sigsetjmp)
	FEEDBACK_ENTER(__sigsetjmp)
1:
	move r2, r0

#define SAVE(r) { ST r2, r ; ADDI_PTR r2, r2, REGSIZE }
	FOR_EACH_CALLEE_SAVED_REG(SAVE)

	mfspr r3, INTERRUPT_CRITICAL_SECTION
	ST r2, r3
	j plt(__sigjmp_save)
	jrp lr   /* Keep the backtracer happy. */
END(__sigsetjmp)
