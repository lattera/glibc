/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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

#ifdef PIC
	.option pic2
#endif
ENTRY (syscall)
	move	v0, a0		/* Load system call number from first arg.  */
	move	a0, a1		/* Move the next three args up a register.  */
	move	a1, a2
	move	a2, a3
     	/* Load the remaining possible args (up to 11) from the stack.  */
#ifdef __mips64
	ld	t0,4*8(sp)
	ld	t1,5*8(sp)
	ld	t2,6*8(sp)
	ld	t3,7*8(sp)
	ld	t4,8*8(sp)
	ld	t5,9*8(sp)
	ld	t6,10*8(sp)
#else
	lw	t0,4*4(sp)
	lw	t1,5*4(sp)
	lw	t2,6*4(sp)
	lw	t3,7*4(sp)
	lw	t4,8*4(sp)
	lw	t5,9*4(sp)
	lw	t6,10*4(sp)
#endif
	syscall			/* Do the system call.  */
     	j ra			/* Return to caller.  */
