/* Copyright (C) 1991, 1992, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
	.text
ENTRY (syscall)
	or	%o0,%g0,%g1
	or	%o1,%g0,%o0
	or	%o2,%g0,%o1
	or	%o3,%g0,%o2
	or	%o4,%g0,%o3
	or	%o5,%g0,%o4
	ta	0x10
	bcc	1f
	nop
	save %sp, -96, %sp
	call __errno_location
	nop
	st %i0,[%o0]
	restore
	retl
	mov -1, %o0
1:
	ret

PSEUDO_END (syscall)
