/* Copyright (C) 1996 Free Software Foundation, Inc.
   Contributed by Richard Henderson (rth@tamu.edu)

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


/* This is the child of the C-with-inline-assembly memcpy posted by
   Martin Ostermann (ost@comnets.rwth-aachen.de).

   This is generally scheduled for the EV5, but whenever necessary and
   possible, the autoswap slotting feature of the EV5 is used so that the
   code lays out nicely for the EV4 as well.  */

#include <alpha/regdef.h>

	.set noreorder

	.text

	.ent copy_fwd_aligned
copy_fwd_aligned:
	.frame sp, 0, ra, 0
	.prologue 0

	/* Aligned forward copy main loop.  On entry to this basic block:
	   t0 == source word waiting to be stored
	   t2 == loop counter
	   a0 == destination pointer
	   a1 == source pointer
	   a2 mod 8 == byte count in final word */
	.align 4
$fa_loop:
	and	t2, 7, t1	# e0    :
	beq	t1, 1f		# .. e1 :

0:	stq_u	t0, 0(a0)	# e0    :
	subq	t1, 1, t1	# .. e1 :
	ldq_u	t0, 8(a1)	# e0    : copy up to seven words
	addq	a0, 8, a0	# .. e1 :
	addq	a1, 8, a1	# e0    :
	bne	t1, 0b		# .. e1 :

1:	bic	t2, 7, t2	# e0    :
	beq	t2, $fa_tail	# .. e1 :

2:	stq_u	t0, 0(a0)	# e0    :
	addq	a0, 64, a0	# .. e1 :
	ldq_u	t3, 8(a1)	# e0    : copy eight words as fast as we can
	ldq_u	t4, 16(a1)	# .. e1 :
	ldq_u	t5, 24(a1)	# e0    :
	ldq_u	t6, 32(a1)	# .. e1 :
	ldq_u	t7, 40(a1)	# e0    :
	ldq_u	t8, 48(a1)	# .. e1 :
	ldq_u	t9, 56(a1)	# e0    :
	ldq_u	t0, 64(a1)	# .. e1 :
	stq_u	t3, -56(a0)	# e0    :
	subq	t2, 8, t2	# .. e1 :
	stq_u	t4, -48(a0)	# e0    :
	addq	a1, 64, a1	# .. e1 :
	stq_u	t5, -40(a0)	# e0    :
	stq_u	t6, -32(a0)	# e0    :
	stq_u	t7, -24(a0)	# e0    :
	stq_u	t8, -16(a0)	# e0    :
	stq_u	t9, -8(a0)	# e0    :
	bne	t2, 2b		# .. e1 :

	/* Take care of a partial word tail.  */
$fa_tail:
	and	a2, 7, t3	# e0    :
	bne	t3, 1f		# .. e1 (zdb)

	/* Aligned copy, aligned tail, final store.  */
	stq_u	t0, 0(a0)
	ret

1:	ldq_u	t1, 0(a0)	# e1    :
	mskql	t0, a2, t0	# .. e1 :
	mskqh	t1, a2, t1	# e0 (stall)
	bis	t0, t1, t0	# e1    :
	stq_u	t0, 0(a0)	# e0    :
	ret			# .. e1 :

	/* This is the actual entry point to this function.  */
	.align 3
$fwd_aligned:
	ldq_u	t0, 0(a1)	# e0    :
	and	a0, 7, t3	# .. e1 :
	addq	a2, t3, a2	# e0    :
	subq	a2, 1, t2	# e1    :
	sra	t2, 3, t2	# e0    :
	beq	t3, $fa_loop	# .. e1 :

	ldq_u	t1, 0(a0)	# e0    :
	beq	t2, $fa_small	# .. e1 :
	mskqh	t0, a0, t0	# e0    :
	mskql	t1, a0, t3	# e0    :
	bis	t0, t3, t0	# e0    :
	br	$fa_loop	# .. e1 :

	/* The move affects exactly one destination word.  */
$fa_small:
	mskqh	t0, a0, t0	# e0    :
	and	a2, 7, t4	# .. e1 :
	mskql	t1, a0, t3	# e0    :
	bne	t4, 1f		# .. e1 :

	or	t0, t3, t0	# e0    :
	unop			#       :
	stq_u	t0, 0(a0)	# e0    :
	ret			# .. e1 :

1:	mskql	t0, a2, t0	# e0    :
	mskqh	t1, a2, t1	# e0    :
	or	t0, t3, t0	# e0    :
	or	t0, t1, t0	# e1    :
	stq_u	t0, 0(a0)	# e0    :
	ret			# .. e1 :

	.end copy_fwd_aligned

	.ent memcpy
	.globl memcpy
	.align 3
memcpy:
	.frame sp, 0, ra, 0
#ifdef PROF
	ldgp	gp, 0(ra)
	lda	AT, _mcount
	jsr	AT, (AT), _mcount
	.prologue 1
#else
	.prologue 0
#endif

	mov	a0, v0
	beq	a2, $zero_length

	/* Are source and destination co-aligned?  */
	xor	a0, a1, t0
	unop
	and	t0, 7, t0
	beq	t0, $fwd_aligned
	br	$fwd_unaligned

	.end memcpy

	.ent copy_fwd_unaligned
copy_fwd_unaligned:
	.frame sp, 0, ra, 0
	.prologue 0

	/* Unaligned forward copy main loop.  On entry to this basic block:
	   t0 == source low word, unshifted
	   t2 == loop counter
	   t7 == last source byte + 1
	   a0 == destination pointer
	   a1 == source pointer
	   a2 mod 8 == byte count in final word */
	.align 4
$fu_loop:
	beq	t2, $fu_tail	# e1    :
	blbc	t2, 0f		# e1    :

	ldq_u	t1, 8(a1)	# e1    : copy one unaligned word
	extql	t0, a1, t3	# .. e0 :
	addq	a1, 8, a1	# e0    :
	addq	a0, 8, a0	# .. e1 :
	extqh	t1, a1, t4	# e0    :
	subq	t2, 1, t2	# .. e1 :
	mov	t1, t0		# e0    :
	or	t3, t4, t3	# .. e1 :
	stq_u	t3, -8(a0)	# e0    :
	beq	t2, $fu_tail	# .. e1 :

0:	ldq_u	t1, 8(a1)	# e1    : copy two unaligned words
	extql	t0, a1, t3	# .. e0 :
	ldq_u	t0, 16(a1)	# e0    :
	subq	t2, 2, t2	# .. e1 :
	extqh	t1, a1, t4	# e0    :
	addq	a0, 16, a0	# .. e1 :
	extql	t1, a1, t5	# e0    :
	or	t3, t4, t3	# .. e1 :
	extqh	t0, a1, t6	# e0    :
	addq	a1, 16, a1	# .. e1 :
	stq_u	t3, -16(a0)	# e0    :
	or	t5, t6, t5	# .. e1 :
	stq_u	t5, -8(a0)	# e0    :
	bne	t2, 0b		# .. e1 :

	/* Take care of a partial words tail.  */
$fu_tail:
	ldq_u	t4, -1(t7)	# e1    :
	extql	t0, a1, t3	# .. e0 :
	extqh	t4, a1, t4	# e0 (stall)
	and	a2, 7, t5	# .. e1 :
	or	t3, t4, t3	# e0    :
	beq	t5, 1f		# .. e1 :

	ldq_u	t1, 0(a0)	# e1    :
	mskql	t3, a2, t3	# .. e0 :
	mskqh	t1, a2, t1	# e0 (stall)
	or	t1, t3, t3	# e1    :

1:	stq_u	t3, 0(a0)	# e0    :
	ret			# .. e1 :

	/* The entry point to the unaligned forward copy.  */
	.align 3
$fwd_unaligned:
	ldq_u	t0, 0(a1)	# e0    : load initial bits of src
	addq	a1, a2, t7	# .. e1 : record last byte + 1 of src
	and	a0, 7, t3	# e0    : find dst misalignment
	addq	a2, t3, a2	# e1    : find number of words affected
	subq	a2, 1, t2	# e0    :
	cmple	a2, 8, t4	# .. e1 : are we dealing with a small block?
	subq	a1, t3, a1	# e0    :
	bne	t4, $fu_small	# .. e1 :
	srl	t2, 3, t2	# e0    :
	beq	t3, $fu_loop	# .. e1 :

	/* Take care of an unaligned dst head.  */
	ldq_u	t5, 0(a0)	# e0    :
	ldq_u	t1, 8(a1)	# .. e1 :
	extql	t0, a1, t3	# e0    :
	addq	a0, 8, a0	# .. e1 :
	extqh	t1, a1, t4	# e0    :
	addq	a1, 8, a1	# .. e1 :
	mskql	t5, a0, t5	# e0    :
	or	t3, t4, t3	# .. e1 :
	mskqh	t3, a0, t3	# e0    :
	subq	t2, 1, t2	# .. e1 :
	or	t3, t5, t3	# e0    :
	mov	t1, t0		# .. e1 :
	stq_u	t3, -8(a0)	# e0    :
	br	$fu_loop	# .. e1 :

	/* The move affects exactly one destination word.  */
	.align 3
$fu_small:
	ldq_u	t2, 0(a0)	# e1    :
	extql	t0, a1, t3	# .. e0 :
	ldq_u	t1, -1(t7)	# e0    :
	and	a2, 7, t8	# .. e1 :
	mskqh	t2, a2, t6	# e0    :
	mskql	t2, a0, t5	# e0    :
	extqh	t1, a1, t4	# e0    :
	cmovne	t8, t6, t8	# .. e1 :
	or	t3, t4, t3	# e0    :
	or	t5, t8, t5	# .. e1 :
	mskqh	t3, a0, t3	# e0    :
	and	a2, 7, t8	# .. e1 :
	mskql	t3, a2, t6	# e0    :
	cmovne	t8, t6, t8	# e1    :
	or	t3, t5, t3	# e0    :
	unop			#       :
	stq_u	t3, 0(a0)	# e0    :

$zero_length:
	ret			# .. e1 :

	.end copy_fwd_unaligned
