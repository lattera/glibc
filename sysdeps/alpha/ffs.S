/* Copyright (C) 1996 Free Software Foundation, Inc.
   Contributed by David Mosberger (davidm@cs.arizona.edu).

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

/* Finds the first bit set in an integer.  Optimized for the Alpha
architecture.  */

#include <sysdep.h>
#ifdef __linux__
# include <alpha/regdef.h>
#else
#include <regdef.h>
#endif

        .set noreorder
        .set noat

ENTRY(ffs)
	.prologue 0

	ldq_u	zero, 0(sp)	# on the 21064, this helps dual-issuing
	addl	a0, zero, a0	# the last insn and reduces the stall
        negq    a0, t0		# due to the srl instruction
        and     a0, t0, t0
	clr	v0
	beq	a0, done

	# now do binary search for first non-zero bit

	zapnot	t0, 0x03, t2
        addq    v0, 16, t3
        cmoveq  t2, t3, v0

	zapnot	t0, 0x05, t2
        addq    v0, 8, t3
        cmoveq  t2, t3, v0

	srl	t0, v0, t0
	addq	v0, 1, v0

        and     t0, 0x0f, t2
        addq    v0, 4, t3
        cmoveq  t2, t3, v0

        and     t0, 0x33, t2
        addq    v0, 2, t3
        cmoveq  t2, t3, v0

        and     t0, 0x55, t2
        addq    v0, 1, t3
        cmoveq  t2, t3, v0

done:   ret

        .end    ffs
