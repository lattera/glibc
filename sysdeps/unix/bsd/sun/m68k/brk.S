/* Copyright (C) 1991, 1992, 1994, 1995, 1997 Free Software Foundation, Inc.
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

#ifndef	SYS_brk
#define	SYS_brk	17
#endif

#ifndef	HAVE_GNU_LD
#define	__end	_end
#endif

.data
.globl ___curbrk
___curbrk:
	.long __end

.text
ENTRY (__brk)
	movel #__end, d0
	cmpl sp@(4), d0
	ble 0f
	movel d0, sp@(4)
0:	pea SYS_brk
	trap #0
	bcs 1f
	movel sp@(4), ___curbrk
	clrl d0
	rts
1:
	jmp syscall_error

weak_alias (__brk, brk)
