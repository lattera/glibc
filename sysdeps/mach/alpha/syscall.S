/* Copyright (C) 1994,97,2002 Free Software Foundation, Inc.
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

ENTRY (syscall)
	mov a0, v0		/* Load system call number from first arg.  */
	mov a1, a0
	mov a2, a1
	mov a3, a2
	mov a4, a3
	mov a5, a4
	/* Load the remaining possible args (up to 11) from the stack.  */
	ldq a5,0(sp)
	ldq t0,8(sp)
	ldq t1,16(sp)
	ldq t2,24(sp)
	ldq t3,32(sp)
	ldq t4,40(sp)
	callsys
	ret
END (syscall)
