/* Copyright (C) 1993, 1995, 1996 Free Software Foundation, Inc.
   Contributed by Brendan Kehoe (brendan@zen.org).

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

.comm errno, 4

	.text
ENTRY(__start)
	lda	sp, -16(sp)
	stq	zero, 8(sp)		/* terminate frame chain */

	br	t0, 1f
1:	ldgp	gp, 0(t0)

	mov	zero, a0		/* establish __fpu_control w/kernel */
	jsr	ra, __setfpucw
	ldgp	gp, 0(ra)

	/* clear out errno. */
	lda	t0, errno
	stl	zero, 0(t0)

	ldl	a0, 16(sp)	/* get argc */
	lda	a1, 24(sp)	/* get argv */

	/* initialize environ: */
	lda	t0, environ
	s8addq	a0, a1, a2
	addq	a2, 0x8, a2
	stq	a2, 0(t0)

#ifndef HAVE_INITFINI
	mov	a0, s0
	mov	a1, s1
	mov	a2, s2

	jsr	ra, __libc_init
	ldgp	gp, 0(ra)

	mov	s0, a0
	mov	s1, a1
	mov	s2, a2

	/* initialize constructors: */
	jsr	ra, __main
	ldgp	gp, 0(ra)

	mov	s0, a0
	mov	s1, a1
	mov	s2, a2
#endif

	jsr	ra, main
	ldgp	gp, 0(ra)

	mov	v0, a0

	lda	pv, exit
	jsr	ra, (pv), 1
	ldgp	gp, 0(ra)

	/* in case exit returns: */

1:	ldi	v0, __NR_exit
	call_pal PAL_callsys
	br	1b

	.end __start


/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0

weak_alias(__data_start, data_start)
