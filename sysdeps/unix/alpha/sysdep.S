/* Copyright (C) 1993, 1996, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Brendan Kehoe (brendan@zen.org).

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
	.globl __errno
__errno = errno
	.globl _errno
_errno = errno

	.text
	.align 2

#ifdef	_LIBC_REENTRANT

	.globl __syscall_error
	.ent __syscall_error
__syscall_error:
	ldgp	gp, 0(pv)
	lda	sp, -16(sp)
	.frame	sp, 16, ra, 0
	stq	ra, 0(sp)
	stq	v0, 8(sp)
	.mask	0x4000001, -16
	.prologue 1

	/* Find our per-thread errno address  */
	jsr	ra, __errno_location

	/* Store the error value.  */
	ldq	t0, 8(sp)
	stl	t0, 0(v0)

	/* And kick back a -1.  */
	ldi	v0, -1

	ldq	ra, 0(sp)
	lda	sp, 16(sp)
	ret
	.end __syscall_error
#else

ENTRY(__syscall_error)
	ldgp	gp, 0(t12)
	.prologue 1

	stl	v0, errno
	lda	v0, -1
	ret
	END(__syscall_error)

#endif /* _LIBC_REENTRANT */
