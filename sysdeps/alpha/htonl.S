/* Copyright (C) 1996 Free Software Foundation, Inc.
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

#include <sysdep.h>

ENTRY(__htonl)
	.prologue 0
	extlh	a0, 5, t1	# t1 = dd000000
	zap	a0, 0xfd, t2	# t2 = 0000cc00
	sll	t2, 5, t2	# t2 = 00198000
	s8addl	t2, t1, t1	# t1 = ddcc0000
	zap	a0, 0xfb, t2	# t2 = 00bb0000
	srl	t2, 8, t2	# t2 = 0000bb00
	extbl	a0, 3, v0	# v0 = 000000aa
	or	t1, v0, v0	# v0 = ddcc00aa
	or	t2, v0, v0	# v0 = ddccbbaa
	ret

	END(__htonl)

strong_alias_asm(__htonl, __ntohl)
weak_alias(__htonl, htonl)
weak_alias(__htonl, ntohl)
