/* Copyright (C) 1993, 1996 Free Software Foundation, Inc.
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
#include <features.h>

	.section .bss
	.globl errno
	.align 2
errno:	.space 4
#ifdef __ELF__
	.type errno, @object
	.size errno, 4
#endif

	.text
	.align 2

#ifdef	_LIBC_REENTRANT

	.globl __syscall_error
	.ent __syscall_error
__syscall_error:
	ldgp	gp, 0(t12)
	lda	sp, -16(sp)
	.frame	sp, 16, ra, 0
	stq	ra, 0(sp)
	stq	v0, 8(sp)
	.mask	0x4000001, -16
	.prologue 1

	/* Find our pre-thread errno address  */
	jsr	ra, __errno_location

	/* Store the error value.  */
	ldl	t0, 8(sp)
	stl	t0, 0(v0)

	/* And kick back a -1.  */
	ldi	v0, -1

	ldq	ra, 0(sp)
	lda	sp, 16(sp)
	ret
	.end __syscall_error

/* A default non-threaded version of __errno_location that just returns
   the address of errno.  */

	.weak	__errno_location
	.ent	__errno_location
__errno_location:
	.frame	sp, 0, ra
	ldgp	gp, 0(t12)
	.mask	0, 0
	.prologue 1

	lda	v0, errno
	ret
	.end __errno_location

#else

ENTRY(__syscall_error)
	ldgp	gp, 0(t12)
	.prologue 1

	stl	v0, errno
	lda	v0, -1
	ret
	END(__syscall_error)

#endif /* _LIBC_REENTRANT */
