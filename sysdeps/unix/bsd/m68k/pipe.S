/* Copyright (C) 1991,1992,1993,1995,1997,2002 Free Software Foundation, Inc.
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

SYSCALL__ (pipe, 1)
#ifdef	__motorola__
	move.l 4(sp), a0
	movem.l d0-d1, (a0)
#else
	movel sp@(4), a0
	moveml d0-d1, a0@
#endif
	clrl d0
	rts

libc_hidden_def (__pipe)
weak_alias (__pipe, pipe)
