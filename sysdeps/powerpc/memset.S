/* Optimized memset implementation for PowerPC.
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

EALIGN(memset,5,1)
/* __ptr_t [r3] memset (__ptr_t s [r3], int c [r4], size_t n [r5]));
   Returns 's'.

   The memset is done in three sizes: byte (8 bits), word (32 bits),
   cache line (256 bits). There is a special case for setting cache lines
   to 0, to take advantage of the dcbz instruction.
   r6:	current address we are storing at
   r7:	number of bytes we are setting now (when aligning)  */

/* take care of case for size <= 4  */
	cmplwi %cr1,%r5,4
	andi.  %r7,%r3,3
	mr     %r6,%r3
	ble-   %cr1,L(small)
/* align to word boundary  */
	cmplwi %cr5,%r5,31
	rlwimi %r4,%r4,8,16,23
	beq+   L(aligned)		# 8th instruction from .align
	mtcrf  0x01,%r3
	subfic %r7,%r7,4
	add    %r6,%r6,%r7
	sub    %r5,%r5,%r7
	bf+    31,0f
	stb    %r4,0(%r3)
	bt     30,L(aligned)
0:	sth    %r4,-2(%r6)		#  16th instruction from .align
/* take care of case for size < 31 */
L(aligned):
	mtcrf  0x01,%r5
	rlwimi %r4,%r4,16,0,15
	ble    %cr5,L(medium)
/* align to cache line boundary...  */
	andi.  %r7,%r6,0x1C
	subfic %r7,%r7,0x20
	beq    L(caligned)
	mtcrf  0x01,%r7
	add    %r6,%r6,%r7
	sub    %r5,%r5,%r7
	cmplwi %cr1,%r7,0x10
	mr     %r8,%r6
	bf     28,1f
	stw    %r4,-4(%r8)
	stwu   %r4,-8(%r8)
1:	blt    %cr1,2f
	stw    %r4,-4(%r8)	# 32nd instruction from .align
	stw    %r4,-8(%r8)
	stw    %r4,-12(%r8)
	stwu   %r4,-16(%r8)
2:	bf     29,L(caligned)
	stw    %r4,-4(%r8)
/* now aligned to a cache line.  */
L(caligned):
	cmplwi %cr1,%r4,0
	clrrwi. %r7,%r5,5
	mtcrf  0x01,%r5		# 40th instruction from .align
	beq    %cr1,L(zloopstart) # special case for clearing memory using dcbz
	srwi   %r0,%r7,5
	mtctr  %r0
	beq    L(medium)	# we may not actually get to do a full line
	clrlwi. %r5,%r5,27
	add    %r6,%r6,%r7
0:	li     %r8,-0x40
	bdz    L(cloopdone)	# 48th instruction from .align

3:	dcbz   %r8,%r6
	stw    %r4,-4(%r6)
	stw    %r4,-8(%r6)
	stw    %r4,-12(%r6)
	stw    %r4,-16(%r6)
	nop			# let 601 fetch last 4 instructions of loop
	stw    %r4,-20(%r6)
	stw    %r4,-24(%r6)	# 56th instruction from .align
	nop			# let 601 fetch first 8 instructions of loop
	stw    %r4,-28(%r6)
	stwu   %r4,-32(%r6)
	bdnz   3b
L(cloopdone):
	stw    %r4,-4(%r6)
	stw    %r4,-8(%r6)
	stw    %r4,-12(%r6)
	stw    %r4,-16(%r6)	# 64th instruction from .align
	stw    %r4,-20(%r6)
	cmplwi %cr1,%r5,16
	stw    %r4,-24(%r6)
	stw    %r4,-28(%r6)
	stwu   %r4,-32(%r6)
	beqlr
	add    %r6,%r6,%r7
	b      L(medium_tail2)	# 72nd instruction from .align

	.align 5
	nop
/* Clear lines of memory in 128-byte chunks.  */
L(zloopstart):
	clrlwi %r5,%r5,27
	mtcrf  0x02,%r7
	srwi.  %r0,%r7,7
	mtctr  %r0
	li     %r7,0x20
	li     %r8,-0x40
	cmplwi %cr1,%r5,16	# 8
	bf     26,0f
	dcbz   0,%r6
	addi   %r6,%r6,0x20
0:	li     %r9,-0x20
	bf     25,1f
	dcbz   0,%r6
	dcbz   %r7,%r6
	addi   %r6,%r6,0x40	# 16
1:	cmplwi %cr5,%r5,0
	beq    L(medium)
L(zloop):
	dcbz   0,%r6
	dcbz   %r7,%r6
	addi   %r6,%r6,0x80
	dcbz   %r8,%r6
	dcbz   %r9,%r6
	bdnz   L(zloop)
	beqlr  %cr5
	b      L(medium_tail2)

	.align 5
L(small):
/* Memset of 4 bytes or less.  */
	cmplwi %cr5,%r5,1
	cmplwi %cr1,%r5,3
	bltlr  %cr5
	stb    %r4,0(%r6)
	beqlr  %cr5
	nop
	stb    %r4,1(%r6)
	bltlr  %cr1
	stb    %r4,2(%r6)
	beqlr  %cr1
	nop
	stb    %r4,3(%r6)
	blr

/* Memset of 0-31 bytes.  */
	.align 5
L(medium):
	cmplwi %cr1,%r5,16
L(medium_tail2):
	add    %r6,%r6,%r5
L(medium_tail):
	bt-    31,L(medium_31t)
	bt-    30,L(medium_30t)
L(medium_30f):
	bt-    29,L(medium_29t)
L(medium_29f):
	bge-   %cr1,L(medium_27t)
	bflr-  28
	stw    %r4,-4(%r6)		# 8th instruction from .align
	stw    %r4,-8(%r6)
	blr

L(medium_31t):
	stbu   %r4,-1(%r6)
	bf-    30,L(medium_30f)
L(medium_30t):
	sthu   %r4,-2(%r6)
	bf-    29,L(medium_29f)
L(medium_29t):
	stwu   %r4,-4(%r6)
	blt-   %cr1,L(medium_27f)	# 16th instruction from .align
L(medium_27t):
	stw    %r4,-4(%r6)
	stw    %r4,-8(%r6)
	stw    %r4,-12(%r6)
	stwu   %r4,-16(%r6)
L(medium_27f):
	bflr-  28
L(medium_28t):
	stw    %r4,-4(%r6)
	stw    %r4,-8(%r6)
	blr
END(memset)
