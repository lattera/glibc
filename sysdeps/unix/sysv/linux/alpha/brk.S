/* Copyright (C) 1993, 1995, 1996 Free Software Foundation, Inc.
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

/* __brk is a special syscall under Linux since it never returns an
error.  Instead, the error condition is indicated by returning the old
break value (instead of the new, requested one).  */

#include <sysdep.h>
#include <errnos.h>

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
LEAF(__brk, 0)
	ldgp	gp, 0(t12)
	.prologue 1

	ldiq	v0, __NR_brk
	call_pal PAL_callsys

	/* Correctly handle the brk(0) query case.  */
	cmoveq	a0, v0, a0

	subq	a0, v0, t0
	bne	t0, error

	/* Update __curbrk and return cleanly.  */
	stq	a0, __curbrk
	mov	zero, v0
	ret

	/* What a horrible way to die.  */
error:	ldi	v0, ENOMEM
	jmp	zero, __syscall_error

	END(__brk)

weak_alias (__brk, brk)
