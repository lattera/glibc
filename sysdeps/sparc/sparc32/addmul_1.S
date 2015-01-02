! SPARC __mpn_addmul_1 -- Multiply a limb vector with a limb and add
! the result to a second limb vector.
!
! Copyright (C) 1992-2015 Free Software Foundation, Inc.
!
! This file is part of the GNU MP Library.
!
! The GNU MP Library is free software; you can redistribute it and/or modify
! it under the terms of the GNU Lesser General Public License as published by
! the Free Software Foundation; either version 2.1 of the License, or (at your
! option) any later version.
!
! The GNU MP Library is distributed in the hope that it will be useful, but
! WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
! or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
! License for more details.
!
! You should have received a copy of the GNU Lesser General Public License
! along with the GNU MP Library; see the file COPYING.LIB.  If not,
! see <http://www.gnu.org/licenses/>.


! INPUT PARAMETERS
! RES_PTR	o0
! S1_PTR	o1
! SIZE		o2
! S2_LIMB	o3

#include <sysdep.h>

ENTRY(__mpn_addmul_1)
	! Make S1_PTR and RES_PTR point at the end of their blocks
	! and put (- 4 x SIZE) in index/loop counter.
	sll	%o2,2,%o2
	add	%o0,%o2,%o4	! RES_PTR in o4 since o0 is retval
	add	%o1,%o2,%o1
	sub	%g0,%o2,%o2

	cmp	%o3,0xfff
	bgu	LOC(large)
	nop

	ld	[%o1+%o2],%o5
	mov	0,%o0
	b	LOC(0)
	 add	%o4,-4,%o4
LOC(loop0):
	addcc	%o5,%g1,%g1
	ld	[%o1+%o2],%o5
	addx	%o0,%g0,%o0
	st	%g1,[%o4+%o2]
LOC(0):	wr	%g0,%o3,%y
	sra	%o5,31,%g2
	and	%o3,%g2,%g2
	andcc	%g1,0,%g1
	mulscc	%g1,%o5,%g1
 	mulscc	%g1,%o5,%g1
 	mulscc	%g1,%o5,%g1
 	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,%o5,%g1
	mulscc	%g1,0,%g1
	sra	%g1,20,%g4
	sll	%g1,12,%g1
 	rd	%y,%g3
	srl	%g3,20,%g3
	or	%g1,%g3,%g1

	addcc	%g1,%o0,%g1
	addx	%g2,%g4,%o0	! add sign-compensation and cy to hi limb
	addcc	%o2,4,%o2	! loop counter
	bne	LOC(loop0)
	 ld	[%o4+%o2],%o5

	addcc	%o5,%g1,%g1
	addx	%o0,%g0,%o0
	retl
	st	%g1,[%o4+%o2]


LOC(large):
	ld	[%o1+%o2],%o5
	mov	0,%o0
	sra	%o3,31,%g4	! g4 = mask of ones iff S2_LIMB < 0
	b	LOC(1)
	 add	%o4,-4,%o4
LOC(loop):
	addcc	%o5,%g3,%g3
	ld	[%o1+%o2],%o5
	addx	%o0,%g0,%o0
	st	%g3,[%o4+%o2]
LOC(1):	wr	%g0,%o5,%y
	and	%o5,%g4,%g2
	andcc	%g0,%g0,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%o3,%g1
	mulscc	%g1,%g0,%g1
	rd	%y,%g3
	addcc	%g3,%o0,%g3
	addx	%g2,%g1,%o0
	addcc	%o2,4,%o2
	bne	LOC(loop)
	 ld	[%o4+%o2],%o5

	addcc	%o5,%g3,%g3
	addx	%o0,%g0,%o0
	retl
	st	%g3,[%o4+%o2]

END(__mpn_addmul_1)
