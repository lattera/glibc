/* memcpy - copy a block from source to destination.  S/390 version.
   Copyright (C) 2000, 2001 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).

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
     %r2 = address of destination memory area
     %r3 = address of source memory area
     %r4 = number of bytes to copy.  */

#include "sysdep.h"
#include "asm-syntax.h"

	.text
ENTRY(memcpy)
	ltr     %r4,%r4
	jz      .L3
	ahi     %r4,-1              # length - 1
	lr      %r1,%r2             # copy destination address
	lr      %r5,%r4
	sra     %r5,8
	jz      .L1
.L0:	mvc     0(256,%r1),0(%r3)   # move in 256 byte chunks
	la      %r1,256(%r1)
	la      %r3,256(%r3)
	brct    %r5,.L0
.L1:	bras    %r5,.L2             # setup base pointer for execute
	mvc     0(1,%r1),0(%r3)     # instruction for execute
.L2:	ex      %r4,0(%r5)          # execute mvc with length ((%r4)&255)+1
.L3:	br      %r14
END(memcpy)
	
