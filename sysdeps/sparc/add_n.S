! sparc __mpn_add_n -- Add two limb vectors of the same length > 0 and store
! sum in a third limb vector.

! Copyright (C) 1992, 1994 Free Software Foundation, Inc.

! This file is part of the GNU MP Library.

! The GNU MP Library is free software; you can redistribute it and/or modify
! it under the terms of the GNU Library General Public License as published by
! the Free Software Foundation; either version 2 of the License, or (at your
! option) any later version.

! The GNU MP Library is distributed in the hope that it will be useful, but
! WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
! or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
! License for more details.

! You should have received a copy of the GNU Library General Public License
! along with the GNU MP Library; see the file COPYING.LIB.  If not, write to
! the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


! INPUT PARAMETERS
! res_ptr	%o0
! s1_ptr	%o1
! s2_ptr	%o2
! size		%o3

#include "sysdep.h"

	.text
	.align	4
	.global	C_SYMBOL_NAME(__mpn_add_n)
C_SYMBOL_NAME(__mpn_add_n):
	ld	[%o1+0],%o4		! read first limb from s1_ptr
	srl	%o3,4,%g1
	ld	[%o2+0],%o5		! read first limb from s2_ptr

	sub	%g0,%o3,%o3
	andcc	%o3,(16-1),%o3
	be	Lzero
	 nop

	sll	%o3,2,%o3		! multiply by 4
	sub	%o0,%o3,%o0		! adjust res_ptr
	sub	%o1,%o3,%o1		! adjust s1_ptr
	sub	%o2,%o3,%o2		! adjust s2_ptr

	mov	%o4,%g2

	sethi	%hi(Lbase),%g3
	or	%g3,%lo(Lbase),%g3
	sll	%o3,2,%o3		! multiply by 4
	jmp	%g3+%o3
	 mov	%o5,%g3

Loop:	addxcc	%g2,%g3,%o3
	add	%o1,64,%o1
	st	%o3,[%o0+60]
	add	%o2,64,%o2
	ld	[%o1+0],%o4
	add	%o0,64,%o0
	ld	[%o2+0],%o5
Lzero:	sub	%g1,1,%g1	! add 0 + 16r limbs (adjust loop counter)
Lbase:	ld	[%o1+4],%g2
	addxcc	%o4,%o5,%o3
	ld	[%o2+4],%g3
	st	%o3,[%o0+0]
	ld	[%o1+8],%o4	! add 15 + 16r limbs
	addxcc	%g2,%g3,%o3
	ld	[%o2+8],%o5
	st	%o3,[%o0+4]
	ld	[%o1+12],%g2	! add 14 + 16r limbs
	addxcc	%o4,%o5,%o3
	ld	[%o2+12],%g3
	st	%o3,[%o0+8]
	ld	[%o1+16],%o4	! add 13 + 16r limbs
	addxcc	%g2,%g3,%o3
	ld	[%o2+16],%o5
	st	%o3,[%o0+12]
	ld	[%o1+20],%g2	! add 12 + 16r limbs
	addxcc	%o4,%o5,%o3
	ld	[%o2+20],%g3
	st	%o3,[%o0+16]
	ld	[%o1+24],%o4	! add 11 + 16r limbs
	addxcc	%g2,%g3,%o3
	ld	[%o2+24],%o5
	st	%o3,[%o0+20]
	ld	[%o1+28],%g2	! add 10 + 16r limbs
	addxcc	%o4,%o5,%o3
	ld	[%o2+28],%g3
	st	%o3,[%o0+24]
	ld	[%o1+32],%o4	! add 9 + 16r limbs
	addxcc	%g2,%g3,%o3
	ld	[%o2+32],%o5
	st	%o3,[%o0+28]
	ld	[%o1+36],%g2	! add 8 + 16r limbs
	addxcc	%o4,%o5,%o3
	ld	[%o2+36],%g3
	st	%o3,[%o0+32]
	ld	[%o1+40],%o4	! add 7 + 16r limbs
	addxcc	%g2,%g3,%o3
	ld	[%o2+40],%o5
	st	%o3,[%o0+36]
	ld	[%o1+44],%g2	! add 6 + 16r limbs
	addxcc	%o4,%o5,%o3
	ld	[%o2+44],%g3
	st	%o3,[%o0+40]
	ld	[%o1+48],%o4	! add 5 + 16r limbs
	addxcc	%g2,%g3,%o3
	ld	[%o2+48],%o5
	st	%o3,[%o0+44]
	ld	[%o1+52],%g2	! add 4 + 16r limbs
	addxcc	%o4,%o5,%o3
	ld	[%o2+52],%g3
	st	%o3,[%o0+48]
	ld	[%o1+56],%o4	! add 3 + 16r limbs
	addxcc	%g2,%g3,%o3
	ld	[%o2+56],%o5
	st	%o3,[%o0+52]
	ld	[%o1+60],%g2	! add 2 + 16r limbs
	addxcc	%o4,%o5,%o3
	ld	[%o2+60],%g3
	st	%o3,[%o0+56]
	addx	%g0,%g0,%o4
	tst	%g1
	bne	Loop
	 subcc	%g0,%o4,%g0	! restore cy (delay slot)

	addxcc	%g2,%g3,%o3
	st	%o3,[%o0+60]	! store most significant limb

	retl
	 addx	%g0,%g0,%o0	! return carry-out from most sign. limb
