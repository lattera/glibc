! SPARC  __udiv_qrnnd division support, used from longlong.h.

! Copyright (C) 1993, 1994 Free Software Foundation, Inc.

! This file is part of the GNU MP Library.

! The GNU MP Library is free software; you can redistribute it and/or modify
! it under the terms of the GNU Lesser General Public License as published by
! the Free Software Foundation; either version 2.1 of the License, or (at your
! option) any later version.

! The GNU MP Library is distributed in the hope that it will be useful, but
! WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
! or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
! License for more details.

! You should have received a copy of the GNU Lesser General Public License
! along with the GNU MP Library; see the file COPYING.LIB.  If not, write to
! the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


! INPUT PARAMETERS
! rem_ptr	o0
! n1		o1
! n0		o2
! d		o3

#include "sysdep.h"

ENTRY(__udiv_qrnnd)
	tst	%o3
	bneg	LOC(largedivisor)
	mov	8,%g1

	b	LOC(p1)
	addxcc	%o2,%o2,%o2

LOC(plop):
	bcc	LOC(n1)
	addxcc	%o2,%o2,%o2
LOC(p1):
	addx	%o1,%o1,%o1
	subcc	%o1,%o3,%o4
	bcc	LOC(n2)
	addxcc	%o2,%o2,%o2
LOC(p2):
	addx	%o1,%o1,%o1
	subcc	%o1,%o3,%o4
	bcc	LOC(n3)
	addxcc	%o2,%o2,%o2
LOC(p3):
	addx	%o1,%o1,%o1
	subcc	%o1,%o3,%o4
	bcc	LOC(n4)
	addxcc	%o2,%o2,%o2
LOC(p4):
	addx	%o1,%o1,%o1
	addcc	%g1,-1,%g1
	bne	LOC(plop)
	subcc	%o1,%o3,%o4
	bcc	LOC(n5)
	addxcc	%o2,%o2,%o2
LOC(p5):
	st	%o1,[%o0]
	retl
	xnor	%g0,%o2,%o0

LOC(nlop):
	bcc	LOC(p1)
	addxcc	%o2,%o2,%o2
LOC(n1):
	addx	%o4,%o4,%o4
	subcc	%o4,%o3,%o1
	bcc	LOC(p2)
	addxcc	%o2,%o2,%o2
LOC(n2):
	addx	%o4,%o4,%o4
	subcc	%o4,%o3,%o1
	bcc	LOC(p3)
	addxcc	%o2,%o2,%o2
LOC(n3):
	addx	%o4,%o4,%o4
	subcc	%o4,%o3,%o1
	bcc	LOC(p4)
	addxcc	%o2,%o2,%o2
LOC(n4):
	addx	%o4,%o4,%o4
	addcc	%g1,-1,%g1
	bne	LOC(nlop)
	subcc	%o4,%o3,%o1
	bcc	LOC(p5)
	addxcc	%o2,%o2,%o2
LOC(n5):
	st	%o4,[%o0]
	retl
	xnor	%g0,%o2,%o0

LOC(largedivisor):
	and	%o2,1,%o5	! %o5 = n0 & 1

	srl	%o2,1,%o2
	sll	%o1,31,%g2
	or	%g2,%o2,%o2	! %o2 = lo(n1n0 >> 1)
	srl	%o1,1,%o1	! %o1 = hi(n1n0 >> 1)

	and	%o3,1,%g2
	srl	%o3,1,%g3	! %g3 = floor(d / 2)
	add	%g3,%g2,%g3	! %g3 = ceil(d / 2)

	b	LOC(Lp1)
	addxcc	%o2,%o2,%o2

LOC(Lplop):
	bcc	LOC(Ln1)
	addxcc	%o2,%o2,%o2
LOC(Lp1):
	addx	%o1,%o1,%o1
	subcc	%o1,%g3,%o4
	bcc	LOC(Ln2)
	addxcc	%o2,%o2,%o2
LOC(Lp2):
	addx	%o1,%o1,%o1
	subcc	%o1,%g3,%o4
	bcc	LOC(Ln3)
	addxcc	%o2,%o2,%o2
LOC(Lp3):
	addx	%o1,%o1,%o1
	subcc	%o1,%g3,%o4
	bcc	LOC(Ln4)
	addxcc	%o2,%o2,%o2
LOC(Lp4):
	addx	%o1,%o1,%o1
	addcc	%g1,-1,%g1
	bne	LOC(Lplop)
	subcc	%o1,%g3,%o4
	bcc	LOC(Ln5)
	addxcc	%o2,%o2,%o2
LOC(Lp5):
	add	%o1,%o1,%o1	! << 1
	tst	%g2
	bne	LOC(Oddp)
	add	%o5,%o1,%o1
	st	%o1,[%o0]
	retl
	xnor	%g0,%o2,%o0

LOC(Lnlop):
	bcc	LOC(Lp1)
	addxcc	%o2,%o2,%o2
LOC(Ln1):
	addx	%o4,%o4,%o4
	subcc	%o4,%g3,%o1
	bcc	LOC(Lp2)
	addxcc	%o2,%o2,%o2
LOC(Ln2):
	addx	%o4,%o4,%o4
	subcc	%o4,%g3,%o1
	bcc	LOC(Lp3)
	addxcc	%o2,%o2,%o2
LOC(Ln3):
	addx	%o4,%o4,%o4
	subcc	%o4,%g3,%o1
	bcc	LOC(Lp4)
	addxcc	%o2,%o2,%o2
LOC(Ln4):
	addx	%o4,%o4,%o4
	addcc	%g1,-1,%g1
	bne	LOC(Lnlop)
	subcc	%o4,%g3,%o1
	bcc	LOC(Lp5)
	addxcc	%o2,%o2,%o2
LOC(Ln5):
	add	%o4,%o4,%o4	! << 1
	tst	%g2
	bne	LOC(Oddn)
	add	%o5,%o4,%o4
	st	%o4,[%o0]
	retl
	xnor	%g0,%o2,%o0

LOC(Oddp):
	xnor	%g0,%o2,%o2
	! q' in %o2. r' in %o1
	addcc	%o1,%o2,%o1
	bcc	LOC(Lp6)
	addx	%o2,0,%o2
	sub	%o1,%o3,%o1
LOC(Lp6):
	subcc	%o1,%o3,%g0
	bcs	LOC(Lp7)
	subx	%o2,-1,%o2
	sub	%o1,%o3,%o1
LOC(Lp7):
	st	%o1,[%o0]
	retl
	mov	%o2,%o0

LOC(Oddn):
	xnor	%g0,%o2,%o2
	! q' in %o2. r' in %o4
	addcc	%o4,%o2,%o4
	bcc	LOC(Ln6)
	addx	%o2,0,%o2
	sub	%o4,%o3,%o4
LOC(Ln6):
	subcc	%o4,%o3,%g0
	bcs	LOC(Ln7)
	subx	%o2,-1,%o2
	sub	%o4,%o3,%o4
LOC(Ln7):
	st	%o4,[%o0]
	retl
	mov	%o2,%o0

END(__udiv_qrnnd)
