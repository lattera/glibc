/* Copyright (C) 1993, 1995 Free Software Foundation, Inc.
   Contributed by David Mosberger (davidm@cs.arizona.edu).

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

/* For compatibility only: a "long" is 64 bits on the Alpha, so
   llseek() isn't really needed.  But there are some programs out
   there who may depend on it being around. 
*/

#include <sysdep.h>

	.text
ENTRY(llseek)
	.prologue 0

	mov	a3, t0		/* save result address */

	sll	a1, 32, a1	/* build a 64 bit ofs out of 32 bit operands */
	zap	a2, 0xf0, a2
	bis	a2, a1, a1

	mov	a4, a2		/* shift down whence */

	ldi	v0, __NR_lseek
	call_pal PAL_callsys
	bne	a3, error

	stq	v0, 0(t0)
	ret

error:	br	gp, 1f
1:	ldgp	gp, 0(gp)
	jmp	zero, syscall_error

	.end llseek
