/* Copyright (C) 1993, 1995 Free Software Foundation, Inc.
   Contributed by David Mosberger (davidm@cs.arizona.edu).

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

/* sigsuspend is a special syscall since it needs to dereference the
   sigset.  */

#include <sysdep.h>

	.text

LEAF(sigsuspend, 0)
	.prologue 0

	ldq	a0, 0(a0)
	ldi	v0, __NR_sigsuspend
	call_pal PAL_callsys
	bne	a3, error
	ret

error:	br	gp, 1f
1:	ldgp	gp, 0(gp)
	jmp	zero, syscall_error

	.end sigsuspend
