/* Copyright (C) 1993 Free Software Foundation, Inc.
   Contributed by Brendan Kehoe (brendan@zen.org).

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <sysdep.h>

#ifndef HAVE_GNU_LD
#define __environ environ
#endif

.comm __environ,	8
.comm errno,		4

!.sdata
!.globl STARTFRM
!STARTFRM = 0

.text
ENTRY(__start)
	lda	sp, -16(sp)
	stq	zero, 8(sp)

	! This branch puts the address of the current insn in t0.
	br	t0, 10f
10:
	! We set the GP register by using the address of the ldgp
	! (what we just put into t0).
	ldgp	gp, 0(t0)

	! get argc
	ldl	a0, 16(sp)

	! get argv
	lda	a1, 24(sp)

	! move ahead to envp
	s8addq	a0, a1, a2
	addq	a2, 0x8, a2

	! Store in environ.
	stq	a2, environ

	! Clear out errno.
!	ldgp	gp, 0(t12)
	stl	zero, errno

	! Call main.
	jsr	ra, main
	ldgp	gp, 0(ra)

	mov	v0, a0

	jsr	ra, exit
	ldgp	gp, 0(ra)

	.end __start
