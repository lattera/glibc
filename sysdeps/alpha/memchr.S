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

/* Finds characters in a memory area.  Optimized for the Alpha
architecture:

      - memory accessed as aligned quadwords only
      - uses cmpbge to compare 8 bytes in parallel
      - does binary search to find 0 byte in last
        quadword (HAKMEM needed 12 instructions to
        do this instead of the 9 instructions that
        binary search needs).

For correctness consider that:

      - only minimum number of quadwords may be accessed
      - the third argument is an unsigned long
*/

#include <sysdep.h>

        .set noreorder
        .set noat

ENTRY(memchr)
#ifdef PROF
	ldgp	gp, 0(pv)
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.prologue 1
#else
	.prologue 0
#endif

	beq	a2, $not_found
        ldq_u   t0, 0(a0)       # load first quadword (a0 may be misaligned)
	addq	a0, a2, t4
	and	a1, 0xff, a1	# a1 = 00000000000000ch
	ldq_u	t5, -1(t4)
	sll	a1,  8, t1	# t1 = 000000000000ch00
	cmpult	a2, 9, t3
	or	t1, a1, a1	# a1 = 000000000000chch
	sll	a1, 16, t1	# t1 = 00000000chch0000
        lda     t2, -1(zero)
	or	t1, a1, a1	# a1 = 00000000chchchch
	sll	a1, 32, t1	# t1 = chchchch00000000
	extql	t0, a0, t6
	or	t1, a1, a1	# a1 = chchchchchchchch

	beq	t3, $first_quad

	extqh	t5, a0, t5
	mov	a0, v0
	or	t6, t5, t0	# t0 = quadword starting at a0

	#
	# Deal with the case where at most 8 bytes remain to be searched
	# in t0.  E.g.:
	#	a2 = 6
	#	t0 = ????c6c5c4c3c2c1
$last_quad:
	negq	a2, t5
	srl	t2, t5, t5	# t5 = mask of a2 bits set
        xor	a1, t0, t0
        cmpbge  zero, t0, t1
	and	t1, t5, t1
        beq     t1, $not_found

$found_it:
	# now, determine which byte matched:
        negq    t1, t2
        and     t1, t2, t1

        and     t1, 0x0f, t0
        addq    v0, 4, t2
        cmoveq  t0, t2, v0

        and     t1, 0x33, t0
        addq    v0, 2, t2
        cmoveq  t0, t2, v0

        and     t1, 0x55, t0
        addq    v0, 1, t2
        cmoveq  t0, t2, v0

$done:	ret


	#
	# Deal with the case where a2 > 8 bytes remain to be
	# searched.  a0 may not be aligned.
	#
$first_quad:
	andnot	a0, 0x7, v0
        insqh   t2, a0, t1	# t1 = 0000ffffffffffff (a0<0:2> ff bytes)
        xor	t0, a1, t0
	or	t0, t1, t0	# t0 = ====ffffffffffff
        cmpbge  zero, t0, t1
        bne     t1, $found_it

	/* at least one byte left to process */

	ldq	t0, 8(v0)
	addq	v0, 8, v0
	/*
	 * Make a2 point to last quad to be accessed (the
	 * last quad may or may not be partial).
	 */
	subq	t4, 1, a2
	andnot	a2, 0x7, a2
	cmpult	v0, a2, t1
	beq	t1, $final

	/* at least two quads remain to be accessed */

	subq	a2, v0, t3	# t3 <- number of quads to be processed in loop
	and	t3, 8, t3	# odd number of quads?
	bne	t3, $odd_quad_count

	/* at least three quads remain to be accessed */

	mov	t0, t3		# move prefetched value into correct register

	.align	3
$unrolled_loop:
	ldq	t0, 8(v0)	# prefetch t0
	xor	a1, t3, t1
	cmpbge	zero, t1, t1
	bne	t1, $found_it

	addq	v0, 8, v0
$odd_quad_count:
	xor	a1, t0, t1
	ldq	t3, 8(v0)	# prefetch t3
	cmpbge	zero, t1, t1
	bne	t1, $found_it

	addq	v0, 8, v0
	cmpult	v0, a2, t5
	bne	t5, $unrolled_loop

	mov	t3, t0		# move prefetched value into t0
$final:	subq	t4, v0, a2	# a2 <- number of bytes left to do
	bne	a2, $last_quad

$not_found:
	mov	zero, v0
	ret

        END(memchr)
