/* Set a block of memory to some byte value.  64 bit S/390 version.
   Copyright (C) 2001, 2003 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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

/* INPUT PARAMETERS
     %r2 = address of memory area
     %r3 = byte to fill memory with
     %r4 = number of bytes to fill.  */

#include "sysdep.h"
#include "asm-syntax.h"

	.text
ENTRY(memset)
	ltgr	%r4,%r4
	jz	.L1
	lgr	%r0,%r2		    # save source address
	lgr	%r1,%r3		    # move pad byte to R1
	lgr	%r3,%r4
	sgr	%r4,%r4		    # no source for MVCLE, only a pad byte
	sgr	%r5,%r5
.L0:	mvcle	%r2,%r4,0(%r1)	    # thats it, MVCLE is your friend
	jo	.L0
	lgr	%r2,%r0		    # return value is source address
.L1:
	br	%r14
END(memset)
libc_hidden_builtin_def (memset)
