/* Copyright (C) 1993, 1995, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger <davidm@cs.arizona.edu>, 1995.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* sigsuspend is a special syscall since it needs to dereference the
   sigset.  This will have to change when we have more than 64 signals.  */

#include <sysdep.h>

	.text

LEAF(__sigsuspend, 0)
#ifdef PROF
	ldgp	gp, 0(pv)
	.set noat
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.set at
	.prologue 1
#else
	.prologue 0
#endif

	ldq	a0, 0(a0)
	ldi	v0, __NR_sigsuspend
	call_pal PAL_callsys
	bne	a3, error
	ret

error:
#ifndef PROF
	br	gp, 1f
1:	ldgp	gp, 0(gp)
#endif
	jmp	zero, __syscall_error

	END(__sigsuspend)

weak_alias(__sigsuspend, sigsuspend)
