/* Copyright (C) 1993-2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

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
	.align	4
	.globl	__brk
	.ent	__brk
	.usepv	__brk, std

	cfi_startproc
__brk:
	ldgp	gp, 0(t12)
	subq	sp, 16, sp
	cfi_adjust_cfa_offset (16)
#ifdef PROF
	.set noat
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.set at
#endif

	/* Save the requested brk across the system call.  */
	stq	a0, 0(sp)

	ldiq	v0, __NR_brk
	call_pal PAL_callsys

	ldq	a0, 0(sp)
	addq	sp, 16, sp
	cfi_adjust_cfa_offset (-16)

	/* Be prepared for an OSF-style brk.  */
	bne	a3, SYSCALL_ERROR_LABEL
	beq	v0, $ok

	/* Correctly handle the brk(0) query case.  */
	cmoveq	a0, v0, a0
	xor	a0, v0, t0
	lda	v0, ENOMEM
	bne	t0, SYSCALL_ERROR_LABEL

	/* Update __curbrk and return cleanly.  */
	lda	v0, 0
$ok:	stq	a0, __curbrk
	ret

PSEUDO_END(__brk)
	cfi_endproc

weak_alias (__brk, brk)
