/* Copyright (C) 1996 Free Software Foundation, Inc.
   Contributed by Richard Henderson (rth@tamu.edu)

This file is part of the GNU C Library.

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

/* Copy a null-terminated string from SRC to DST.  Return a pointer
   to the null-terminator in the source.  */

#include <sysdep.h>

	.text

ENTRY(__stpcpy)
	ldgp	gp, 0(pv)
	.prologue 1

	jsr	t9, __stxcpy	# do the work of the copy

	and	t8, 0xf0, t2	# binary search for byte offset of the
	and	t8, 0xcc, t1	# last byte written.
	and	t8, 0xaa, t0
	andnot	a0, 7, a0
	cmovne	t2, 4, t2
	cmovne	t1, 2, t1
	cmovne	t0, 1, t0
	addq	a0, t2, v0
	addq	t0, t1, t0
	addq	v0, t0, v0

	ret

	END(__stpcpy)

weak_alias (__stpcpy, stpcpy)
