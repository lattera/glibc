/* Optimized strchr implementation for PowerPC.
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

/* See strlen.s for comments on how this works.  */

/* char * [r3] strchr (const char *s [r3] , int c [r4] )

   r0:	a temporary
   r3:	our return result.
   r4:	byte we're looking for, spread over the whole word
   r5:	the current word
   r6:	the constant 0xfefefeff (-0x01010101)
   r7:	the constant 0x7f7f7f7f
   r8:	pointer to the current word.
   r9:	a temporary
   r10:	the number of bits we should ignore in the first word
   r11:	a mask with the bits to ignore set to 0
   r12:	a temporary  */
ENTRY(strchr)
	rlwimi %r4,%r4,8,16,23
	li   %r11,-1
	rlwimi %r4,%r4,16,0,15
	lis  %r6,0xfeff
	lis  %r7,0x7f7f
	clrrwi %r8,%r3,2
	addi %r7,%r7,0x7f7f
	addi %r6,%r6,0xfffffeff
	rlwinm %r10,%r3,3,27,28
/* Test the first (partial?) word.  */
	lwz  %r5,0(%r8)
	srw  %r11,%r11,%r10
	orc  %r5,%r5,%r11
	add  %r0,%r6,%r5
	nor  %r9,%r7,%r5
	and. %r0,%r0,%r9
	xor  %r12,%r4,%r5
	orc  %r12,%r12,%r11
	b    L(loopentry)

/* The loop.  */

L(loop):lwzu %r5,4(%r8)
	and. %r0,%r0,%r9
/* Test for 0.  */
	add  %r0,%r6,%r5
	nor  %r9,%r7,%r5
	bne  L(foundit)
	and. %r0,%r0,%r9
/* Start test for the bytes we're looking for.  */
	xor  %r12,%r4,%r5
L(loopentry):
	add  %r0,%r6,%r12
	nor  %r9,%r7,%r12
	beq  L(loop)
/* There is a zero byte in the word, but may also be a matching byte (either
   before or after the zero byte).  In fact, we may be looking for a
   zero byte, in which case we return a match.  We guess that this hasn't
   happened, though.  */
L(missed):
	and. %r0,%r0,%r9
	li   %r3,0
	beqlr
/* It did happen. Decide which one was first...
   I'm not sure if this is actually faster than a sequence of
   rotates, compares, and branches (we use it anyway because it's shorter).  */
	and  %r6,%r7,%r5
	or   %r11,%r7,%r5
	and  %r0,%r7,%r12
	or   %r10,%r7,%r12
	add  %r6,%r6,%r7
	add  %r0,%r0,%r7
	nor  %r5,%r11,%r6
	nor  %r9,%r10,%r0
	cmplw %r5,%r9
	bgtlr
	cntlzw %r4,%r9
	srwi %r4,%r4,3
	add  %r3,%r8,%r4
	blr

L(foundit):
	and  %r0,%r7,%r12
	or   %r10,%r7,%r12
	add  %r0,%r0,%r7
	nor  %r9,%r10,%r0
	cntlzw %r4,%r9
	subi %r8,%r8,4
	srwi %r4,%r4,3
	add  %r3,%r8,%r4
	blr
END(strchr)

weak_alias(strchr,index)
