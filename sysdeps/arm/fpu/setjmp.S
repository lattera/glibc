/* setjmp for ARM.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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

ENTRY (__sigsetjmp)
	/* Save registers */
	sfmea	f4, 4, [r0]!
	stmia	r0, {v1-v6, sl, fp, sp, lr}

	/* Restore pointer to jmp_buf */
	sub	r0, r0, #48

	/* Make a tail call to __sigjmp_save; it takes the same args.  */
	B	PLTJMP(C_SYMBOL_NAME(__sigjmp_save))
END (__sigsetjmp)

hidden_def (__sigsetjmp)
