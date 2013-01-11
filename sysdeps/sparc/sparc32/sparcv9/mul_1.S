! SPARC v9 32-bit mpn_mul_1.
!
! Copyright 2010-2013 Free Software Foundation, Inc.
!
! This file is part of the GNU MP Library.
!
! The GNU MP Library is free software; you can redistribute it and/or modify
! it under the terms of the GNU Lesser General Public License as published
! by the Free Software Foundation; either version 3 of the License, or (at
! your option) any later version.

! The GNU MP Library is distributed in the hope that it will be useful, but
! WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
! or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
! License for more details.
!
! You should have received a copy of the GNU Lesser General Public License
! along with the GNU MP Library.  If not, see http://www.gnu.org/licenses/.

! INPUT PARAMETERS
! res_ptr	%o0
! s1_ptr	%o1
! size		%o2
! s2_limb	%o3

#include <sysdep.h>

ENTRY(__mpn_mul_1)
	srl	%o2, 0, %o2
	srl	%o3, 0, %o3
	subcc	%o2, 1, %o2
	be	.Lfinal_one
	 clr	%o5

.Ltop:
	lduw	[%o1+0], %g1
	lduw	[%o1+4], %g2
	mulx	%g1, %o3, %g3
	add	%o1, 8, %o1
	mulx	%g2, %o3, %o4
	sub	%o2, 2, %o2
	add	%o0, 8, %o0
	add	%o5, %g3, %g3
	stw	%g3, [%o0-8]
	srlx	%g3, 32, %o5
	add	%o5, %o4, %o4
	stw	%o4, [%o0-4]
	brgz	%o2, .Ltop
	 srlx	%o4, 32, %o5

	brlz,pt	%o2, .Ldone
	 nop

.Lfinal_one:
	lduw	[%o1+0], %g1
	mulx	%g1, %o3, %g3
	add	%o5, %g3, %g3
	stw	%g3, [%o0+0]
	srlx	%g3, 32, %o5

.Ldone:
	retl
	 mov	%o5, %o0
END(__mpn_mul_1)
