 # Shift a limb left, low level routine.
 # Copyright (C) 1996, 1997 Free Software Foundation, Inc.
 # This file is part of the GNU C Library.
 #
 # The GNU C Library is free software; you can redistribute it and/or
 # modify it under the terms of the GNU Library General Public License as
 # published by the Free Software Foundation; either version 2 of the
 # License, or (at your option) any later version.
 #
 # The GNU C Library is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; without even the implied warranty of
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 # Library General Public License for more details.
 #
 # You should have received a copy of the GNU Library General Public
 # License along with the GNU C Library; see the file COPYING.LIB.  If not,
 # write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 # Boston, MA 02111-1307, USA.

 # mp_limb_t mpn_lshift (mp_ptr wp, mp_srcptr up, mp_size_t usize,
 #			 unsigned int cnt)

	.align 3
	.globl __mpn_lshift
	.type	 __mpn_lshift,@function
__mpn_lshift:
	mtctr	%r5		# copy size into CTR
	cmplwi	%cr0,%r5,16	# is size < 16
	slwi	%r0,%r5,2
	add	%r7,%r3,%r0	# make r7 point at end of res
	add	%r4,%r4,%r0	# make r4 point at end of s1
	lwzu	%r11,-4(%r4)	# load first s1 limb
	subfic	%r8,%r6,32
	srw	%r3,%r11,%r8	# compute function return value
	bge	%cr0,Lbig	# branch if size >= 16

	bdz	Lend1

Loop:	lwzu	%r10,-4(%r4)
	slw	%r9,%r11,%r6
	srw	%r12,%r10,%r8
	or	%r9,%r9,%r12
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slw	%r9,%r10,%r6
	srw	%r12,%r11,%r8
	or	%r9,%r9,%r12
	stwu	%r9,-4(%r7)
	bdnz	Loop
	b	Lend1

 # Guaranteed not to succeed.
LBoom:	tweq    %r0,%r0

 # We imitate a case statement, by using (yuk!) fixed-length code chunks,
 # of size 4*12 bytes.  We have to do this (or something) to make this PIC.
Lbig:	mflr    %r9
	bltl    %cr0,LBoom      # Never taken, only used to set LR.
	slwi    %r10,%r6,4
	mflr    %r12
	add     %r10,%r12,%r10
	slwi	%r8,%r6,5
	add     %r10,%r8,%r10
	mtctr   %r10
	addi	%r5,%r5,-1
	mtlr    %r9
	bctr

Lend1:	slw	%r0,%r11,%r6
	stw	%r0,-4(%r7)
	blr

	mtctr	%r5
Loop1:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,1
	inslwi	%r9,%r10,1,31
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,1
	inslwi	%r9,%r11,1,31
	stwu	%r9,-4(%r7)
	bdnz	Loop1
	b	Lend1

	mtctr	%r5
Loop2:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,2
	inslwi	%r9,%r10,2,30
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,2
	inslwi	%r9,%r11,2,30
	stwu	%r9,-4(%r7)
	bdnz	Loop2
	b	Lend1

	mtctr	%r5
Loop3:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,3
	inslwi	%r9,%r10,3,29
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,3
	inslwi	%r9,%r11,3,29
	stwu	%r9,-4(%r7)
	bdnz	Loop3
	b	Lend1

	mtctr	%r5
Loop4:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,4
	inslwi	%r9,%r10,4,28
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,4
	inslwi	%r9,%r11,4,28
	stwu	%r9,-4(%r7)
	bdnz	Loop4
	b	Lend1

	mtctr	%r5
Loop5:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,5
	inslwi	%r9,%r10,5,27
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,5
	inslwi	%r9,%r11,5,27
	stwu	%r9,-4(%r7)
	bdnz	Loop5
	b	Lend1

	mtctr	%r5
Loop6:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,6
	inslwi	%r9,%r10,6,26
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,6
	inslwi	%r9,%r11,6,26
	stwu	%r9,-4(%r7)
	bdnz	Loop6
	b	Lend1

	mtctr	%r5
Loop7:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,7
	inslwi	%r9,%r10,7,25
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,7
	inslwi	%r9,%r11,7,25
	stwu	%r9,-4(%r7)
	bdnz	Loop7
	b	Lend1

	mtctr	%r5
Loop8:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,8
	inslwi	%r9,%r10,8,24
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,8
	inslwi	%r9,%r11,8,24
	stwu	%r9,-4(%r7)
	bdnz	Loop8
	b	Lend1

	mtctr	%r5
Loop9:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,9
	inslwi	%r9,%r10,9,23
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,9
	inslwi	%r9,%r11,9,23
	stwu	%r9,-4(%r7)
	bdnz	Loop9
	b	Lend1

	mtctr	%r5
Loop10:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,10
	inslwi	%r9,%r10,10,22
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,10
	inslwi	%r9,%r11,10,22
	stwu	%r9,-4(%r7)
	bdnz	Loop10
	b	Lend1

	mtctr	%r5
Loop11:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,11
	inslwi	%r9,%r10,11,21
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,11
	inslwi	%r9,%r11,11,21
	stwu	%r9,-4(%r7)
	bdnz	Loop11
	b	Lend1

	mtctr	%r5
Loop12:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,12
	inslwi	%r9,%r10,12,20
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,12
	inslwi	%r9,%r11,12,20
	stwu	%r9,-4(%r7)
	bdnz	Loop12
	b	Lend1

	mtctr	%r5
Loop13:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,13
	inslwi	%r9,%r10,13,19
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,13
	inslwi	%r9,%r11,13,19
	stwu	%r9,-4(%r7)
	bdnz	Loop13
	b	Lend1

	mtctr	%r5
Loop14:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,14
	inslwi	%r9,%r10,14,18
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,14
	inslwi	%r9,%r11,14,18
	stwu	%r9,-4(%r7)
	bdnz	Loop14
	b	Lend1

	mtctr	%r5
Loop15:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,15
	inslwi	%r9,%r10,15,17
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,15
	inslwi	%r9,%r11,15,17
	stwu	%r9,-4(%r7)
	bdnz	Loop15
	b	Lend1

	mtctr	%r5
Loop16:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,16
	inslwi	%r9,%r10,16,16
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,16
	inslwi	%r9,%r11,16,16
	stwu	%r9,-4(%r7)
	bdnz	Loop16
	b	Lend1

	mtctr	%r5
Loop17:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,17
	inslwi	%r9,%r10,17,15
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,17
	inslwi	%r9,%r11,17,15
	stwu	%r9,-4(%r7)
	bdnz	Loop17
	b	Lend1

	mtctr	%r5
Loop18:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,18
	inslwi	%r9,%r10,18,14
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,18
	inslwi	%r9,%r11,18,14
	stwu	%r9,-4(%r7)
	bdnz	Loop18
	b	Lend1

	mtctr	%r5
Loop19:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,19
	inslwi	%r9,%r10,19,13
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,19
	inslwi	%r9,%r11,19,13
	stwu	%r9,-4(%r7)
	bdnz	Loop19
	b	Lend1

	mtctr	%r5
Loop20:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,20
	inslwi	%r9,%r10,20,12
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,20
	inslwi	%r9,%r11,20,12
	stwu	%r9,-4(%r7)
	bdnz	Loop20
	b	Lend1

	mtctr	%r5
Loop21:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,21
	inslwi	%r9,%r10,21,11
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,21
	inslwi	%r9,%r11,21,11
	stwu	%r9,-4(%r7)
	bdnz	Loop21
	b	Lend1

	mtctr	%r5
Loop22:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,22
	inslwi	%r9,%r10,22,10
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,22
	inslwi	%r9,%r11,22,10
	stwu	%r9,-4(%r7)
	bdnz	Loop22
	b	Lend1

	mtctr	%r5
Loop23:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,23
	inslwi	%r9,%r10,23,9
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,23
	inslwi	%r9,%r11,23,9
	stwu	%r9,-4(%r7)
	bdnz	Loop23
	b	Lend1

	mtctr	%r5
Loop24:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,24
	inslwi	%r9,%r10,24,8
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,24
	inslwi	%r9,%r11,24,8
	stwu	%r9,-4(%r7)
	bdnz	Loop24
	b	Lend1

	mtctr	%r5
Loop25:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,25
	inslwi	%r9,%r10,25,7
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,25
	inslwi	%r9,%r11,25,7
	stwu	%r9,-4(%r7)
	bdnz	Loop25
	b	Lend1

	mtctr	%r5
Loop26:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,26
	inslwi	%r9,%r10,26,6
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,26
	inslwi	%r9,%r11,26,6
	stwu	%r9,-4(%r7)
	bdnz	Loop26
	b	Lend1

	mtctr	%r5
Loop27:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,27
	inslwi	%r9,%r10,27,5
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,27
	inslwi	%r9,%r11,27,5
	stwu	%r9,-4(%r7)
	bdnz	Loop27
	b	Lend1

	mtctr	%r5
Loop28:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,28
	inslwi	%r9,%r10,28,4
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,28
	inslwi	%r9,%r11,28,4
	stwu	%r9,-4(%r7)
	bdnz	Loop28
	b	Lend1

	mtctr	%r5
Loop29:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,29
	inslwi	%r9,%r10,29,3
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,29
	inslwi	%r9,%r11,29,3
	stwu	%r9,-4(%r7)
	bdnz	Loop29
	b	Lend1

	mtctr	%r5
Loop30:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,30
	inslwi	%r9,%r10,30,2
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,30
	inslwi	%r9,%r11,30,2
	stwu	%r9,-4(%r7)
	bdnz	Loop30
	b	Lend1

	mtctr	%r5
Loop31:	lwzu	%r10,-4(%r4)
	slwi	%r9,%r11,31
	inslwi	%r9,%r10,31,1
	stwu	%r9,-4(%r7)
	bdz	Lend2
	lwzu	%r11,-4(%r4)
	slwi	%r9,%r10,31
	inslwi	%r9,%r11,31,1
	stwu	%r9,-4(%r7)
	bdnz	Loop31
	b	Lend1

Lend2:	slw	%r0,%r10,%r6
	stw	%r0,-4(%r7)
	blr
