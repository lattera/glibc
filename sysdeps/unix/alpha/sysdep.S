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
#define _ERRNO_H
#include <errnos.h>

	.section .bss
	.globl errno
	.align 2
errno:	.space 4
#ifdef __ELF__
	.type errno, @object
	.size errno, 4
#endif

	.text
LEAF(__syscall_error, 0)
	ldgp	gp, 0(t12)
	.prologue 1

	/* Store return value in errno... */
	stl	v0, errno

	/* And just kick back a -1.  */
	ldi	v0, -1
	ret

	END(__syscall_error)
