/* Copyright (C) 1994 Free Software Foundation, Inc.
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

ENTRY (syscall)
	.frame	sp,0,ra
	move	v0, a0		/* Load system call number from first arg.  */
	move	a0, a1		/* Move the next three args up a register.  */
	move	a1, a2
	move	a2, a3
     	/* Load the remaining possible args (up to 11) from the stack.  */
	lw	t0,16(sp)
	lw	t1,20(sp)
	lw	t2,24(sp)
	lw	t3,28(sp)
	lw	t4,32(sp)
	lw	t5,36(sp)
	lw	t6,40(sp)
	syscall			/* Do the system call.  */
     	j ra			/* Return to caller.  */
	.end	syscall
