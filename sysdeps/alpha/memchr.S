/* Copyright (C) 1996, 2000, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger (davidm@cs.arizona.edu).

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

/* Finds characters in a memory area.  Optimized for the Alpha:

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

ENTRY(__memchr)
#ifdef PROF
	ldgp	gp, 0(pv)
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.prologue 1
#else
	.prologue 0
#endif

	# Hack -- if someone passes in (size_t)-1, hoping to just
	# search til the end of the address space, we will overflow
	# below when we find the address of the last byte.  Given
	# that we will never have a 56-bit address space, cropping
	# the length is the easiest way to avoid trouble.
	zap	a2, 0x80, t4	#-e0	:

	beq	a2, $not_found	# .. e1 :
        ldq_u   t0, 0(a0)       # e1	: load first quadword
	insbl	a1, 1, t1	# .. e0 : t1 = 000000000000ch00
	and	a1, 0xff, a1	#-e0    : a1 = 00000000000000ch
	cmpult	a2, 9, t3	# .. e1 :
	or	t1, a1, a1	# e0    : a1 = 000000000000chch
        lda     t2, -1(zero)	# .. e1 :
	sll	a1, 16, t1	#-e0    : t1 = 00000000chch0000
	addq	a0, t4, t4	# .. e1 :
	or	t1, a1, a1	# e1    : a1 = 00000000chchchch
	unop			#	:
	sll	a1, 32, t1	#-e0    : t1 = chchchch00000000
	or	t1, a1, a1	# e1	: a1 = chchchchchchchch
	extql	t0, a0, t6	# e0    :
	beq	t3, $first_quad	# .. e1 :

	ldq_u	t5, -1(t4)	#-e1	: eight or less bytes to search
	extqh	t5, a0, t5	# .. e0 :
	mov	a0, v0		# e0	:
	or	t6, t5, t0	# .. e1 : t0 = quadword starting at a0

	# Deal with the case where at most 8 bytes remain to be searched
	# in t0.  E.g.:
	#	a2 = 6
	#	t0 = ????c6c5c4c3c2c1
$last_quad:
	negq	a2, t5		#-e0	:
        xor	a1, t0, t0	# .. e1 :
	srl	t2, t5, t5	# e0    : t5 = mask of a2 bits set
        cmpbge  zero, t0, t1	# .. e1 :
	and	t1, t5, t1	#-e0	:
        beq     t1, $not_found	# .. e1 :

$found_it:
	# Now, determine which byte matched:
        negq    t1, t2		# e0	:
        and     t1, t2, t1	# e1	:

        and     t1, 0x0f, t0	#-e0	:
        addq    v0, 4, t2	# .. e1 :
        cmoveq  t0, t2, v0	# e0	:

        addq    v0, 2, t2	# .. e1 :
        and     t1, 0x33, t0	#-e0	:
        cmoveq  t0, t2, v0	# .. e1 :

        and     t1, 0x55, t0	# e0	:
        addq    v0, 1, t2	# .. e1 :
        cmoveq  t0, t2, v0	#-e0	:

$done:	ret			# .. e1 :

	# Deal with the case where a2 > 8 bytes remain to be
	# searched.  a0 may not be aligned.
	.align 4
$first_quad:
	andnot	a0, 0x7, v0	#-e1	:
        insqh   t2, a0, t1	# .. e0	: t1 = 0000ffffffffffff (a0<0:2> ff)
        xor	t0, a1, t0	# e0	:
	or	t0, t1, t0	# e1	: t0 = ====ffffffffffff
        cmpbge  zero, t0, t1	#-e0	:
        bne     t1, $found_it	# .. e1 :

	# At least one byte left to process.

	ldq	t0, 8(v0)	# e0	:
	subq	t4, 1, a2	# .. e1 :
	addq	v0, 8, v0	#-e0	:

	# Make a2 point to last quad to be accessed (the
	# last quad may or may not be partial).

	andnot	a2, 0x7, a2	# .. e1 :
	cmpult	v0, a2, t1	# e0	:
	beq	t1, $final	# .. e1 :

	# At least two quads remain to be accessed.

	subq	a2, v0, t3	#-e0	: t3 <- nr quads to be processed
	and	t3, 8, t3	# e1	: odd number of quads?
	bne	t3, $odd_quad_count # e1 :

	# At least three quads remain to be accessed

	mov	t0, t3		# e0	: move prefetched value to correct reg

	.align	4
$unrolled_loop:
	ldq	t0, 8(v0)	#-e0	: prefetch t0
	xor	a1, t3, t1	# .. e1 :
	cmpbge	zero, t1, t1	# e0	:
	bne	t1, $found_it	# .. e1 :

	addq	v0, 8, v0	#-e0	:
$odd_quad_count:
	xor	a1, t0, t1	# .. e1 :
	ldq	t3, 8(v0)	# e0	: prefetch t3
	cmpbge	zero, t1, t1	# .. e1 :
	addq	v0, 8, t5	#-e0	:
	bne	t1, $found_it	# .. e1	:

	cmpult	t5, a2, t5	# e0	:
	addq	v0, 8, v0	# .. e1 :
	bne	t5, $unrolled_loop #-e1 :

	mov	t3, t0		# e0	: move prefetched value into t0
$final:	subq	t4, v0, a2	# .. e1	: a2 <- number of bytes left to do
	bne	a2, $last_quad	# e1	:

$not_found:
	mov	zero, v0	#-e0	:
	ret			# .. e1 :

        END(__memchr)

weak_alias (__memchr, memchr)
#if !__BOUNDED_POINTERS__
weak_alias (__memchr, __ubp_memchr)
#endif
libc_hidden_builtin_def (memchr)
