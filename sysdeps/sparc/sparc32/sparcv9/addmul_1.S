! SPARC v9 32-bit mpn_addmul_1.
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
! res_ptr	%i0
! s1_ptr	%i1
! size		%i2
! s2_limb	%i3

#include <sysdep.h>

ENTRY(__mpn_addmul_1)
	save	%sp, -96, %sp
	srl	%i2, 0, %o4
	srl	%i3, 0, %g1
	subcc	%o4, 1, %o4
	be	.Lfinal_one
	 clr	%o5

.Ltop:
	lduw	[%i1+0], %l0
	lduw	[%i0+0], %l2
	lduw	[%i1+4], %l1
	lduw	[%i0+4], %l3
	mulx	%l0, %g1, %g3
	add	%i1, 8, %i1
	mulx	%l1, %g1, %o3
	sub	%o4, 2, %o4
	add	%i0, 8, %i0
	add	%l2, %g3, %g3
	add	%o5, %g3, %g3
	stw	%g3, [%i0-8]
	srlx	%g3, 32, %o5
	add	%l3, %o3, %o3
	add	%o5, %o3, %o3
	stw	%o3, [%i0-4]
	brgz	%o4, .Ltop
	 srlx	%o3, 32, %o5

	brlz,pt	%o4, .Ldone
	 nop

.Lfinal_one:
	lduw	[%i1+0], %l0
	lduw	[%i0+0], %l2
	mulx	%l0, %g1, %g3
	add	%l2, %g3, %g3
	add	%o5, %g3, %g3
	stw	%g3, [%i0+0]
	srlx	%g3, 32, %o5

.Ldone:
	jmpl	%i7 + 8, %g0
	 restore %o5, 0, %o0
END(__mpn_addmul_1)
