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

ENTRY(strcpy)
	ldgp	gp, 0(pv)
	.prologue 1

	mov	a0, v0		# set up return value
	jsr	t9, __stxcpy	# do the copy
	ret

	END(strcpy)
