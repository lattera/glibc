/* Copyright (C) 1993, 1995, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Brendan Kehoe <brendan@zen.org>, 1993.

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

/* __brk is a special syscall under Linux since it never returns an
   error.  Instead, the error condition is indicated by returning the old
   break value (instead of the new, requested one).  */

#include <sysdep.h>
#define _ERRNO_H
#include <bits/errno.h>

#ifdef PIC
.section .bss
	.align 3
	.globl __curbrk
__curbrk: .skip 8
	.type __curbrk,@object
	.size __curbrk,8
#else
.comm __curbrk, 8
#endif

	.text
LEAF(__brk, 8)
	ldgp	gp, 0(t12)
	subq	sp, 8, sp
#ifdef PROF
	.set noat
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.set at
#endif
	.prologue 1

	/* Save the requested brk across the system call.  */
	stq	a0, 0(sp)

	ldiq	v0, __NR_brk
	call_pal PAL_callsys

	ldq	a0, 0(sp)

	/* Be prepared for an OSF-style brk.  */
	bne	a3, $err1
	beq	v0, $ok

	/* Correctly handle the brk(0) query case.  */
	cmoveq	a0, v0, a0
	xor	a0, v0, t0
	bne	t0, $err0

	/* Update __curbrk and return cleanly.  */
	mov	zero, v0
$ok:	stq	a0, __curbrk
	addq	sp, 8, sp
	ret

	/* What a horrible way to die.  */
$err0:	ldi	v0, ENOMEM
$err1:	addq	sp, 8, sp
	SYSCALL_ERROR_HANDLER

	END(__brk)

weak_alias (__brk, brk)
