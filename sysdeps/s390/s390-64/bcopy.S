/* bcopy -- copy a block from source to destination.  64 bit S/390 version.
   This file is part of the GNU C Library.
   Copyright (C) 2000-2018 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).

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
     %r2 = address of source
     %r3 = address of destination
     %r4 = number of bytes to copy.  */

#include "sysdep.h"
#include "asm-syntax.h"

	.text
ENTRY(__bcopy)
	ltgr	%r1,%r4		    # zero bcopy ?
	jz	.L4
	clgr	%r2,%r3		    # check against destructive overlap
	jnl	.L0
	algr	%r1,%r2
	clgr	%r1,%r3
	jh	.L7
.L0:	aghi	%r4,-1		    # length - 1
	srlg	%r1,%r4,8
	ltgr	%r1,%r1             # < 256 bytes to move ?
	jz	.L2
	cghi    %r1,255             # > 1MB to move ?
	jh      .L5
.L1:	mvc	0(256,%r3),0(%r2)   # move in 256 byte chunks
	la	%r2,256(%r2)
	la	%r3,256(%r3)
	brctg	%r1,.L1
.L2:	bras	%r1,.L3		    # setup base pointer for execute
	mvc	0(1,%r3),0(%r2)	    # instruction for execute
.L3:	ex	%r4,0(%r1)	    # execute mvc with length ((%r4)&255)+1
.L4:	br	%r14
	# data copies > 1MB are faster with mvcle.
.L5:	aghi    %r4,1               # length + 1
	lgr	%r5,%r4	            # source length
	lgr	%r4,%r2             # source address
	lgr	%r2,%r3             # set destination
	lgr	%r3,%r5             # destination length = source length
.L6:	mvcle	%r2,%r4,0           # thats it, MVCLE is your friend
	jo	.L6
	br	%r14
.L7:				    # destructive overlay, can not use mvcle
	lgr	%r1,%r2		    # bcopy is called with source,dest
	lgr	%r2,%r3		    # memmove with dest,source! Oh, well...
	lgr	%r3,%r1
	jg	HIDDEN_BUILTIN_JUMPTARGET(memmove)

END(__bcopy)

#ifndef NO_WEAK_ALIAS
weak_alias (__bcopy, bcopy)
#endif

