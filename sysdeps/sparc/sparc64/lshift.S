/* SPARC v9 __mpn_lshift --

   Copyright (C) 1996 Free Software Foundation, Inc.

   This file is part of the GNU MP Library.

   The GNU MP Library is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at your
   option) any later version.

   The GNU MP Library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
   License for more details.

   You should have received a copy of the GNU Library General Public License
   along with the GNU MP Library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
   MA 02111-1307, USA.  */

#include <sysdep.h>

/* INPUT PARAMETERS
   res_ptr	%i0
   src_ptr	%i1
   size		%i2
   cnt		%i3  */

ENTRY(__mpn_lshift)
	save	%sp, -192, %sp

	sllx	%i2,3,%g1
	add	%i1,%g1,%i1	! make %i1 point at end of src
	ldx	[%i1-8],%g2	! load first limb
	sub	%g0,%i3,%i5	! negate shift count
	add	%i0,%g1,%i0	! make %i0 point at end of res
	add	%i2,-1,%i2
	and	%i2,4-1,%l4	! number of limbs in first loop
	srlx	%g2,%i5,%g1	! compute function result
	brz,pn	%l4,.L0		! if multiple of 4 limbs, skip first loop
	mov	%g1,%l1

	sub	%i2,%l4,%i2	! adjust count for main loop

.Loop0:	ldx	[%i1-16],%g3
	add	%i0,-8,%i0
	add	%i1,-8,%i1
	add	%l4,-1,%l4
	sllx	%g2,%i3,%i4
	srlx	%g3,%i5,%g1
	mov	%g3,%g2
	or	%i4,%g1,%i4
	brnz,pt	%l4,.Loop0
	 stx	%i4,[%i0+0]

.L0:	brz,pn	%i2,.Lend
	 nop

.Loop:	ldx	[%i1-16],%g3
	add	%i0,-32,%i0
	add	%i2,-4,%i2
	sllx	%g2,%i3,%i4
	srlx	%g3,%i5,%g1

	ldx	[%i1-24],%g2
	sllx	%g3,%i3,%l4
	or	%i4,%g1,%i4
	stx	%i4,[%i0+24]
	srlx	%g2,%i5,%g1

	ldx	[%i1-32],%g3
	sllx	%g2,%i3,%i4
	or	%l4,%g1,%l4
	stx	%l4,[%i0+16]
	srlx	%g3,%i5,%g1

	ldx	[%i1-40],%g2
	sllx	%g3,%i3,%l4
	or	%i4,%g1,%i4
	stx	%i4,[%i0+8]
	srlx	%g2,%i5,%g1

	add	%i1,-32,%i1
	or	%l4,%g1,%l4
	brnz,pt	%i2,.Loop
	 stx	%l4,[%i0+0]

.Lend:	sllx	%g2,%i3,%g2
	stx	%g2,[%i0-8]

	mov	%l1,%i0
	jmpl	%i7+8, %g0
	 restore

END(__mpn_lshift)
