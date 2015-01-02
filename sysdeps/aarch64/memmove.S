/* Copyright (C) 2012-2015 Free Software Foundation, Inc.

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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

/* Assumptions:
 *
 * ARMv8-a, AArch64
 * Unaligned accesses
 */

/* Parameters and result.  */
#define dstin	x0
#define src	x1
#define count	x2
#define tmp1	x3
#define tmp1w	w3
#define tmp2	x4
#define tmp2w	w4
#define tmp3	x5
#define tmp3w	w5
#define dst	x6

#define A_l	x7
#define A_h	x8
#define B_l	x9
#define B_h	x10
#define C_l	x11
#define C_h	x12
#define D_l	x13
#define D_h	x14

ENTRY_ALIGN (memmove, 6)

	cmp	dstin, src
	b.lo	L(downwards)
	add	tmp1, src, count
	cmp	dstin, tmp1
	b.hs	memcpy		/* No overlap.  */

	/* Upwards move with potential overlap.
	 * Need to move from the tail backwards.  SRC and DST point one
	 * byte beyond the remaining data to move.  */
	add	dst, dstin, count
	add	src, src, count
	cmp	count, #64
	b.ge	L(mov_not_short_up)

	/* Deal with small moves quickly by dropping straight into the
	 * exit block.  */
L(tail63up):
	/* Move up to 48 bytes of data.  At this point we only need the
	 * bottom 6 bits of count to be accurate.  */
	ands	tmp1, count, #0x30
	b.eq	L(tail15up)
	sub	dst, dst, tmp1
	sub	src, src, tmp1
	cmp	tmp1w, #0x20
	b.eq	1f
	b.lt	2f
	ldp	A_l, A_h, [src, #32]
	stp	A_l, A_h, [dst, #32]
1:
	ldp	A_l, A_h, [src, #16]
	stp	A_l, A_h, [dst, #16]
2:
	ldp	A_l, A_h, [src]
	stp	A_l, A_h, [dst]
L(tail15up):
	/* Move up to 15 bytes of data.  Does not assume additional data
	 * being moved.  */
	tbz	count, #3, 1f
	ldr	tmp1, [src, #-8]!
	str	tmp1, [dst, #-8]!
1:
	tbz	count, #2, 1f
	ldr	tmp1w, [src, #-4]!
	str	tmp1w, [dst, #-4]!
1:
	tbz	count, #1, 1f
	ldrh	tmp1w, [src, #-2]!
	strh	tmp1w, [dst, #-2]!
1:
	tbz	count, #0, 1f
	ldrb	tmp1w, [src, #-1]
	strb	tmp1w, [dst, #-1]
1:
	RET

L(mov_not_short_up):
	/* We don't much care about the alignment of DST, but we want SRC
	 * to be 128-bit (16 byte) aligned so that we don't cross cache line
	 * boundaries on both loads and stores.  */
	ands	tmp2, src, #15		/* Bytes to reach alignment.  */
	b.eq	2f
	sub	count, count, tmp2
	/* Move enough data to reach alignment; unlike memcpy, we have to
	 * be aware of the overlap, which means we can't move data twice.  */
	tbz	tmp2, #3, 1f
	ldr	tmp1, [src, #-8]!
	str	tmp1, [dst, #-8]!
1:
	tbz	tmp2, #2, 1f
	ldr	tmp1w, [src, #-4]!
	str	tmp1w, [dst, #-4]!
1:
	tbz	tmp2, #1, 1f
	ldrh	tmp1w, [src, #-2]!
	strh	tmp1w, [dst, #-2]!
1:
	tbz	tmp2, #0, 1f
	ldrb	tmp1w, [src, #-1]!
	strb	tmp1w, [dst, #-1]!
1:

	/* There may be less than 63 bytes to go now.  */
	cmp	count, #63
	b.le	L(tail63up)
2:
	subs	count, count, #128
	b.ge	L(mov_body_large_up)
	/* Less than 128 bytes to move, so handle 64 here and then jump
	 * to the tail.  */
	ldp	A_l, A_h, [src, #-64]!
	ldp	B_l, B_h, [src, #16]
	ldp	C_l, C_h, [src, #32]
	ldp	D_l, D_h, [src, #48]
	stp	A_l, A_h, [dst, #-64]!
	stp	B_l, B_h, [dst, #16]
	stp	C_l, C_h, [dst, #32]
	stp	D_l, D_h, [dst, #48]
	tst	count, #0x3f
	b.ne	L(tail63up)
	RET

	/* Critical loop.  Start at a new Icache line boundary.  Assuming
	 * 64 bytes per line this ensures the entire loop is in one line.  */
	.p2align 6
L(mov_body_large_up):
	/* There are at least 128 bytes to move.  */
	ldp	A_l, A_h, [src, #-16]
	ldp	B_l, B_h, [src, #-32]
	ldp	C_l, C_h, [src, #-48]
	ldp	D_l, D_h, [src, #-64]!
1:
	stp	A_l, A_h, [dst, #-16]
	ldp	A_l, A_h, [src, #-16]
	stp	B_l, B_h, [dst, #-32]
	ldp	B_l, B_h, [src, #-32]
	stp	C_l, C_h, [dst, #-48]
	ldp	C_l, C_h, [src, #-48]
	stp	D_l, D_h, [dst, #-64]!
	ldp	D_l, D_h, [src, #-64]!
	subs	count, count, #64
	b.ge	1b
	stp	A_l, A_h, [dst, #-16]
	stp	B_l, B_h, [dst, #-32]
	stp	C_l, C_h, [dst, #-48]
	stp	D_l, D_h, [dst, #-64]!
	tst	count, #0x3f
	b.ne	L(tail63up)
	RET

L(downwards):
	/* For a downwards move we can safely use memcpy provided that
	 * DST is more than 16 bytes away from SRC.  */
	sub	tmp1, src, #16
	cmp	dstin, tmp1
	b.ls	memcpy		/* May overlap, but not critically.  */

	mov	dst, dstin	/* Preserve DSTIN for return value.  */
	cmp	count, #64
	b.ge	L(mov_not_short_down)

	/* Deal with small moves quickly by dropping straight into the
	 * exit block.  */
L(tail63down):
	/* Move up to 48 bytes of data.  At this point we only need the
	 * bottom 6 bits of count to be accurate.  */
	ands	tmp1, count, #0x30
	b.eq	L(tail15down)
	add	dst, dst, tmp1
	add	src, src, tmp1
	cmp	tmp1w, #0x20
	b.eq	1f
	b.lt	2f
	ldp	A_l, A_h, [src, #-48]
	stp	A_l, A_h, [dst, #-48]
1:
	ldp	A_l, A_h, [src, #-32]
	stp	A_l, A_h, [dst, #-32]
2:
	ldp	A_l, A_h, [src, #-16]
	stp	A_l, A_h, [dst, #-16]
L(tail15down):
	/* Move up to 15 bytes of data.  Does not assume additional data
	   being moved.  */
	tbz	count, #3, 1f
	ldr	tmp1, [src], #8
	str	tmp1, [dst], #8
1:
	tbz	count, #2, 1f
	ldr	tmp1w, [src], #4
	str	tmp1w, [dst], #4
1:
	tbz	count, #1, 1f
	ldrh	tmp1w, [src], #2
	strh	tmp1w, [dst], #2
1:
	tbz	count, #0, 1f
	ldrb	tmp1w, [src]
	strb	tmp1w, [dst]
1:
	RET

L(mov_not_short_down):
	/* We don't much care about the alignment of DST, but we want SRC
	 * to be 128-bit (16 byte) aligned so that we don't cross cache line
	 * boundaries on both loads and stores.  */
	neg	tmp2, src
	ands	tmp2, tmp2, #15		/* Bytes to reach alignment.  */
	b.eq	2f
	sub	count, count, tmp2
	/* Move enough data to reach alignment; unlike memcpy, we have to
	 * be aware of the overlap, which means we can't move data twice.  */
	tbz	tmp2, #3, 1f
	ldr	tmp1, [src], #8
	str	tmp1, [dst], #8
1:
	tbz	tmp2, #2, 1f
	ldr	tmp1w, [src], #4
	str	tmp1w, [dst], #4
1:
	tbz	tmp2, #1, 1f
	ldrh	tmp1w, [src], #2
	strh	tmp1w, [dst], #2
1:
	tbz	tmp2, #0, 1f
	ldrb	tmp1w, [src], #1
	strb	tmp1w, [dst], #1
1:

	/* There may be less than 63 bytes to go now.  */
	cmp	count, #63
	b.le	L(tail63down)
2:
	subs	count, count, #128
	b.ge	L(mov_body_large_down)
	/* Less than 128 bytes to move, so handle 64 here and then jump
	 * to the tail.  */
	ldp	A_l, A_h, [src]
	ldp	B_l, B_h, [src, #16]
	ldp	C_l, C_h, [src, #32]
	ldp	D_l, D_h, [src, #48]
	stp	A_l, A_h, [dst]
	stp	B_l, B_h, [dst, #16]
	stp	C_l, C_h, [dst, #32]
	stp	D_l, D_h, [dst, #48]
	tst	count, #0x3f
	add	src, src, #64
	add	dst, dst, #64
	b.ne	L(tail63down)
	RET

	/* Critical loop.  Start at a new cache line boundary.  Assuming
	 * 64 bytes per line this ensures the entire loop is in one line.  */
	.p2align 6
L(mov_body_large_down):
	/* There are at least 128 bytes to move.  */
	ldp	A_l, A_h, [src, #0]
	sub	dst, dst, #16		/* Pre-bias.  */
	ldp	B_l, B_h, [src, #16]
	ldp	C_l, C_h, [src, #32]
	ldp	D_l, D_h, [src, #48]!	/* src += 64 - Pre-bias.  */
1:
	stp	A_l, A_h, [dst, #16]
	ldp	A_l, A_h, [src, #16]
	stp	B_l, B_h, [dst, #32]
	ldp	B_l, B_h, [src, #32]
	stp	C_l, C_h, [dst, #48]
	ldp	C_l, C_h, [src, #48]
	stp	D_l, D_h, [dst, #64]!
	ldp	D_l, D_h, [src, #64]!
	subs	count, count, #64
	b.ge	1b
	stp	A_l, A_h, [dst, #16]
	stp	B_l, B_h, [dst, #32]
	stp	C_l, C_h, [dst, #48]
	stp	D_l, D_h, [dst, #64]
	add	src, src, #16
	add	dst, dst, #64 + 16
	tst	count, #0x3f
	b.ne	L(tail63down)
	RET
END (memmove)

libc_hidden_builtin_def (memmove)
