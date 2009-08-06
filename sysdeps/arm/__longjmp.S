/* longjmp for ARM.
   Copyright (C) 1997, 1998, 2009 Free Software Foundation, Inc.
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

#include <sysdep.h>
#define _SETJMP_H
#define _ASM
#include <bits/setjmp.h>

/* __longjmp(jmpbuf, val) */

ENTRY (__longjmp)
	mov	ip, r0
	movs	r0, r1		/* get the return value in place */
	moveq	r0, #1		/* can't let setjmp() return zero! */

#ifdef CHECK_SP
	ldr	r4, [ip, #32]
	CHECK_SP (r4)
#endif
	LOADREGS(ia, ip, {v1-v6, sl, fp, sp, pc})
END (__longjmp)
