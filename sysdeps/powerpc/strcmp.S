/* Optimized strcmp implementation for PowerPC.
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

EALIGN(strcmp,4,0)
/* int [r3] strcmp (const char *p1 [r3], const char *p2 [r4])  */

/* General register assignments:
   r0:	temporary
   r3:	pointer to previous word in s1
   r4:	pointer to previous word in s2
   r5:	current word from s1
   r6:	current word from s2
   r7:	0xfefefeff
   r8:	0x7f7f7f7f
   r9:	~(word in s1 | 0x7f7f7f7f)  */

/* Register assignments in the prologue:
   r10:	low 2 bits of p2-p1
   r11:	mask to orc with r5/r6  */

	or    %r0,%r4,%r3
	clrlwi. %r0,%r0,30
	lis   %r7,0xfeff
	bne   L(unaligned)

	lwz   %r5,0(%r3)
	lwz   %r6,0(%r4)
	lis   %r8,0x7f7f
	addi  %r7,%r7,-0x101
	addi  %r8,%r8,0x7f7f
	b     1f

0:	lwzu  %r5,4(%r3)
	bne   %cr1,L(different)
	lwzu  %r6,4(%r4)
1:	add   %r0,%r7,%r5
	nor   %r9,%r8,%r5
	and.  %r0,%r0,%r9
	cmpw  %cr1,%r5,%r6
	beq+  0b
L(endstring):
/* OK. We've hit the end of the string. We need to be careful that
   we don't compare two strings as different because of gunk beyond
   the end of the strings...  */
	and   %r0,%r8,%r5
	beq   %cr1,L(equal)
	add   %r0,%r0,%r8
	xor.  %r10,%r5,%r6
	andc  %r9,%r9,%r0
	blt-  L(highbit)
	cntlzw %r10,%r10
	cntlzw %r9,%r9
	addi  %r9,%r9,7
	cmpw  %cr1,%r9,%r10
	sub   %r3,%r5,%r6
	bgelr+ %cr1
L(equal):
	li    %r3,0
	blr

L(different):
	lwz   %r5,-4(%r3)
	xor.  %r10,%r5,%r6
	sub   %r3,%r5,%r6
	bgelr+
L(highbit):
	ori   %r3,%r6,1
	blr


/* Oh well.  In this case, we just do a byte-by-byte comparison.  */
	.align 4
L(unaligned):
	lbz   %r5,0(%r3)
	lbz   %r6,0(%r4)
	b     1f

0:	lbzu  %r5,1(%r3)
	bne-  4f
	lbzu  %r6,1(%r4)
1:	cmpwi %cr1,%r5,0
	beq-  %cr1,3f
	cmpw  %r5,%r6
	bne-  3f
	lbzu  %r5,1(%r3)
	lbzu  %r6,1(%r4)
	cmpwi %cr1,%r5,0
	cmpw  %r5,%r6
	bne+  %cr1,0b
3:	sub   %r3,%r5,%r6
	blr
4:	lbz   %r5,-1(%r3)
	sub   %r3,%r5,%r6
	blr
END(strcmp)
