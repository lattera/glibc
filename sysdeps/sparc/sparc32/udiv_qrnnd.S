! SPARC  __udiv_qrnnd division support, used from longlong.h.
!
! Copyright (C) 1993, 1994, 1997 Free Software Foundation, Inc.
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
! along with the GNU MP Library; see the file COPYING.LIB.  If not, write to
! the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
!
! Added PIC support - May/96, Miguel de Icaza
!
! INPUT PARAMETERS
! rem_ptr	i0
! n1		i1
! n0		i2
! d		i3

#include <sysdep.h>
#undef ret	/* Kludge for glibc */

#ifdef PIC
	.text
#else
	.section .rodata,#alloc
#endif
	.align	8

	.type two_to_32,@object
	.size two_to_32,8
two_to_32:
	.double	0r4294967296

	.type two_to_31,@object
	.size two_to_31,8
two_to_31:
	.double	0r2147483648

	.text
ENTRY(__udiv_qrnnd)
	!#PROLOGUE# 0
	save	%sp,-104,%sp
	!#PROLOGUE# 1
	st	%i1,[%fp-8]
	ld	[%fp-8],%f10
#ifdef PIC
LOC(base):
	call	1f
	fitod	%f10,%f4
1:	ldd	[%o7-(LOC(base)-two_to_32)],%f8
#else
	sethi	%hi(two_to_32),%o7
	fitod	%f10,%f4
	ldd	[%o7+%lo(two_to_32)],%f8
#endif
	cmp	%i1,0
	bge	LOC(248)
	mov	%i0,%i5
	faddd	%f4,%f8,%f4
LOC(248):
	st	%i2,[%fp-8]
	ld	[%fp-8],%f10
	fmuld	%f4,%f8,%f6
	cmp	%i2,0
	bge	LOC(249)
	fitod	%f10,%f2
	faddd	%f2,%f8,%f2
LOC(249):
	st	%i3,[%fp-8]
	faddd	%f6,%f2,%f2
	ld	[%fp-8],%f10
	cmp	%i3,0
	bge	LOC(250)
	fitod	%f10,%f4
	faddd	%f4,%f8,%f4
LOC(250):
	fdivd	%f2,%f4,%f2
#ifdef PIC
	ldd	[%o7-(LOC(base)-two_to_31)],%f4
#else
	sethi	%hi(two_to_31),%o7
	ldd	[%o7+%lo(two_to_31)],%f4
#endif
	fcmped	%f2,%f4
	nop
	fbge,a	LOC(251)
	fsubd	%f2,%f4,%f2
	fdtoi	%f2,%f2
	st	%f2,[%fp-8]
	b	LOC(252)
	ld	[%fp-8],%i4
LOC(251):
	fdtoi	%f2,%f2
	st	%f2,[%fp-8]
	ld	[%fp-8],%i4
	sethi	%hi(-2147483648),%g2
	xor	%i4,%g2,%i4
LOC(252):
	wr	%g0,%i4,%y
	sra	%i3,31,%g2
	and	%i4,%g2,%g2
	andcc	%g0,0,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,%i3,%g1
	mulscc	%g1,0,%g1
	add	%g1,%g2,%i0
	rd	%y,%g3
	subcc	%i2,%g3,%o7
	subxcc	%i1,%i0,%g0
	be	LOC(253)
	cmp	%o7,%i3

	add	%i4,-1,%i0
	add	%o7,%i3,%o7
	st	%o7,[%i5]
	ret
	restore
LOC(253):
	blu	LOC(246)
	mov	%i4,%i0
	add	%i4,1,%i0
	sub	%o7,%i3,%o7
LOC(246):
	st	%o7,[%i5]
	ret
	restore

END(__udiv_qrnnd)
