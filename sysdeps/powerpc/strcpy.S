/* Optimized strcpy implementation for PowerPC.
   Copyright (C) 1997 Free Software Foundation, Inc.
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

#include <sysdep.h>

/* See strlen.s for comments on how the end-of-string testing works.  */

EALIGN(strcpy,4,0)
/* char * [r3] strcpy (char *dest [r3], const char *src [r4])  */

/* General register assignments:
   r0:	temporary
   r3:	saved `dest'
   r4:	pointer to previous word in src
   r5:	pointer to previous word in dest
   r6:	current word from src
   r7:	0xfefefeff
   r8:	0x7f7f7f7f
   r9:	~(word in src | 0x7f7f7f7f)
   r10:	alternate word from src.  */

	or    %r0,%r4,%r3
	clrlwi. %r0,%r0,30
	addi  %r5,%r3,-4
	bne   L(unaligned)

	lis   %r7,0xfeff
	lis   %r8,0x7f7f
	lwz   %r6,0(%r4)
	addi  %r7,%r7,-0x101
	addi  %r8,%r8,0x7f7f
	b     2f

0:	lwzu  %r10,4(%r4)
	stwu  %r6,4(%r5)
	add   %r0,%r7,%r10
	nor   %r9,%r8,%r10
	and.  %r0,%r0,%r9
	bne-  1f
	lwzu  %r6,4(%r4)
	stwu  %r10,4(%r5)
2:	add   %r0,%r7,%r6
	nor   %r9,%r8,%r6
	and.  %r0,%r0,%r9
	beq+  0b

	mr    %r10,%r6
/* We've hit the end of the string.  Do the rest byte-by-byte.  */
1:	rlwinm. %r0,%r10,8,24,31
	stb   %r0,4(%r5)
	beqlr-
	rlwinm. %r0,%r10,16,24,31
	stb   %r0,5(%r5)
	beqlr-
	rlwinm. %r0,%r10,24,24,31
	stb   %r0,6(%r5)
	beqlr-
	stb   %r10,7(%r5)
	blr

/* Oh well.  In this case, we just do a byte-by-byte copy.  */
	.align 4
	nop
L(unaligned):
	lbz   %r6,0(%r4)
	addi  %r5,%r3,-1
	cmpwi %r6,0
	beq-  2f

0:	lbzu  %r10,1(%r4)
	stbu  %r6,1(%r5)
	cmpwi %r10,0
	beq-  1f
	nop		/* Let 601 load start of loop.  */
	lbzu  %r6,1(%r4)
	stbu  %r10,1(%r5)
	cmpwi %r6,0
	bne+  0b
2:	stb   %r6,1(%r5)
	blr
1:	stb   %r10,1(%r5)
	blr

END(strcpy)
