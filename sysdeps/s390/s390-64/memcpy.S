/* Set a block of memory to some byte value.  64 bit S/390 version.
   Copyright (C) 2001 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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

/* INPUT PARAMETERS
     %r2 = address to destination memory area
     %r3 = address to source memory area
     %r4 = number of bytes to copy.  */

#include "sysdep.h"
#include "asm-syntax.h"

	.text
ENTRY(memcpy)
	ltgr	%r5,%r4
	jz	.L1
	lgr	%r4,%r3		    # %r4/%r5 = source ptr/len
	lgr	%r3,%r5		    # %r2/%r3 = dest ptr/len
	lgr	%r0,%r2		    # save source address
.L0:	mvcle	%r2,%r4,0	    # thats it, MVCLE is your friend
	jo	.L0
	lgr	%r2,%r0		    # return value is source address
.L1:
	br	%r14
END(memset)
