/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   Contributed by David Mosberger (davidm@cs.arizona.edu).
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

/* __bb_init_func is invoked at the beginning of each function, before
   any registers have been saved.  It is therefore safe to use any
   caller-saved (call-used) registers (except for argument registers
   a1-a5). */

#include <sysdep.h>

/*
 * These offsets should match with "struct bb" declared in gcc/libgcc2.c.
 */
#define	ZERO_WORD	0x00
#define NEXT		0x20

	.set	noat
	.set	noreorder

ENTRY(__bb_init_func)
	.prologue 0

	ldq	t0, ZERO_WORD(a0)	/* t0 <- blocks->zero_word */
	beq	t0, init		/* not initialized yet -> */
	ret

END(__bb_init_func)

	.ent init
init:
	.frame	sp, 0x38, ra, 0
	subq	sp, 0x38, sp
	.prologue 0

	stq	pv, 0x30(sp)
	br	pv, 1f
1:	ldgp	gp, 0(pv)

	ldiq	t1, __bb_head
	lda	t3, _gmonparam
	ldq	t2, 0(t1)
	ldl	t3, 0(t3)		/* t3 = _gmonparam.state */
	lda	t0, 1
	stq	t0, ZERO_WORD(a0)	/* blocks->zero_word = 1 */
	stq	t2, NEXT(a0)		/* blocks->next = __bb_head */
	stq	a0, 0(t1)
	bne	t2, $leave
	beq	t3, $leave		/* t3 == GMON_PROF_ON? yes -> */

	/* also need to initialize destructor: */
	stq	ra, 0x00(sp)
	lda	a0, __bb_exit_func
	stq	a1, 0x08(sp)
	lda	pv, atexit
	stq	a2, 0x10(sp)
	stq	a3, 0x18(sp)
	stq	a4, 0x20(sp)
	stq	a5, 0x28(sp)
	jsr	ra, (pv), atexit
	ldq	ra, 0x00(sp)
	ldq	a1, 0x08(sp)
	ldq	a2, 0x10(sp)
	ldq	a3, 0x18(sp)
	ldq	a4, 0x20(sp)
	ldq	a5, 0x28(sp)

$leave:	ldq	pv, 0x30(sp)
	addq	sp, 0x38, sp
	ret

	.end init
