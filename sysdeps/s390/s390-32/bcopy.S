/* bcopy -- copy a block from source to destination.  S/390 version.
   This file is part of the GNU C Library.
   Copyright (C) 2000-2016 Free Software Foundation, Inc.
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
	ltr     %r1,%r4             # zero bcopy ?
	jz      .L4
        clr     %r2,%r3             # check against destructive overlap
        jnl     .L0
        alr     %r1,%r2
        clr     %r1,%r3
        jh      .L7
.L0:	ahi     %r4,-1              # length - 1
	lr      %r1,%r4
	srl     %r1,8
	ltr     %r1,%r1             # < 256 bytes to move ?
	jz      .L2
	chi     %r1,255             # > 1MB to move ?
	jh      .L5
.L1:	mvc     0(256,%r3),0(%r2)   # move in 256 byte chunks
	la      %r2,256(%r2)
	la      %r3,256(%r3)
	brct    %r1,.L1
.L2:	bras    %r1,.L3             # setup base pointer for execute
	mvc     0(1,%r3),0(%r2)     # instruction for execute
.L3:	ex      %r4,0(%r1)          # execute mvc with length ((%r4)&255)+1
.L4:	br      %r14

	# data copies > 1MB are faster with mvcle.
.L5:	ahi     %r4,1               # length + 1
	lr      %r5,%r4	            # source length
	lr	%r4,%r2             # source address
	lr	%r2,%r3             # set destination
	lr	%r3,%r5             # destination length = source length
.L6:	mvcle	%r2,%r4,0           # thats it, MVCLE is your friend
	jo	.L6
	br	%r14
.L7:                                # destructive overlay, can not use mvcle
        lr     %r1,%r2              # bcopy is called with source,dest
        lr     %r2,%r3              # memmove with dest,source! Oh, well...
        lr     %r3,%r1
        basr   %r1,0
.L8:
#ifdef PIC
        al     %r1,.L9-.L8(%r1)     # get address of global offset table
                                    # load address of memmove
        l      %r1,memmove@GOT(%r1)
        br     %r1
.L9:    .long  _GLOBAL_OFFSET_TABLE_-.L8
#else
        al     %r1,.L9-.L8(%r1)     # load address of memmove
        br     %r1                  # jump to memmove
.L9:    .long  memmove-.L8
#endif

END(__bcopy)

#ifndef NO_WEAK_ALIAS
weak_alias (__bcopy, bcopy)
#endif

