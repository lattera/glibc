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

/* Assumptions:
 *
 * ARMv8-a, AArch64
 * Unaligned accesses
 *
 */

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

#include <sysdep.h>

ENTRY_ALIGN (memcpy, 6)

	mov	dst, dstin
	cmp	count, #64
	b.ge	L(cpy_not_short)
	cmp	count, #15
	b.le	L(tail15tiny)

	/* Deal with small copies quickly by dropping straight into the
	 * exit block.  */
L(tail63):
	/* Copy up to 48 bytes of data.  At this point we only need the
	 * bottom 6 bits of count to be accurate.  */
	ands	tmp1, count, #0x30
	b.eq	L(tail15)
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

L(tail15):
	ands	count, count, #15
	beq	1f
	add	src, src, count
	ldp	A_l, A_h, [src, #-16]
	add	dst, dst, count
	stp	A_l, A_h, [dst, #-16]
1:
	RET

L(tail15tiny):
	/* Copy up to 15 bytes of data.  Does not assume additional data
	   being copied.  */
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

L(cpy_not_short):
	/* We don't much care about the alignment of DST, but we want SRC
	 * to be 128-bit (16 byte) aligned so that we don't cross cache line
	 * boundaries on both loads and stores.  */
	neg	tmp2, src
	ands	tmp2, tmp2, #15		/* Bytes to reach alignment.  */
	b.eq	2f
	sub	count, count, tmp2
	/* Copy more data than needed; it's faster than jumping
	 * around copying sub-Quadword quantities.  We know that
	 * it can't overrun.  */
	ldp	A_l, A_h, [src]
	add	src, src, tmp2
	stp	A_l, A_h, [dst]
	add	dst, dst, tmp2
	/* There may be less than 63 bytes to go now.  */
	cmp	count, #63
	b.le	L(tail63)
2:
	subs	count, count, #128
	b.ge	L(cpy_body_large)
	/* Less than 128 bytes to copy, so handle 64 here and then jump
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
	b.ne	L(tail63)
	RET

	/* Critical loop.  Start at a new cache line boundary.  Assuming
	 * 64 bytes per line this ensures the entire loop is in one line.  */
	.p2align 6
L(cpy_body_large):
	/* There are at least 128 bytes to copy.  */
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
	b.ne	L(tail63)
	RET
END (memcpy)
libc_hidden_builtin_def (memcpy)
