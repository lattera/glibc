/* Copyright (C) 2017-2018 Free Software Foundation, Inc.

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

/* Assumptions: ARMv8-a, AArch64, falkor, unaligned accesses.  */

#define dstin	x0
#define src	x1
#define count	x2
#define dstlen	x3
#define dst	x3
#define srcend	x4
#define dstend	x5
#define A_l	x6
#define A_lw	w6
#define A_h	x7
#define A_hw	w7
#define B_l	x8
#define B_lw	w8
#define B_h	x9
#define C_l	x10
#define C_h	x11
#define D_l	x12
#define D_h	x13
#define E_l	src
#define E_h	count
#define F_l	srcend
#define F_h	dst
#define tmp1	x14

/* Alias with A_l and A_h to train the prefetcher.  */
#define Q_l	x22
#define Q_h	x23

/* RATIONALE:

   The copy has 4 distinct parts:
   * Small copies of 16 bytes and under
   * Medium sized copies of 17-96 bytes
   * Large copies where the source address is higher than the destination
     (forward copies)
   * Large copies where the destination address is higher than the source
     (copy backward, or move).

   We use only two registerpairs x6,x7 and x22,x23 for the copies and copy 32
   bytes at a time to correctly train the hardware prefetcher for better
   throughput.  */
ENTRY_ALIGN (__memmove_falkor, 6)

	sub	tmp1, dstin, src
	add	srcend, src, count
	add	dstend, dstin, count
	cmp	count, 96
	ccmp	tmp1, count, 2, hi
	b.lo	L(move_long)

	cmp	count, 16
	b.ls	L(copy16)
	cmp	count, 96
	b.hi	L(copy_long)

	/* Medium copies: 17..96 bytes.  */
	sub	tmp1, count, 1
	ldp	A_l, A_h, [src]
	tbnz	tmp1, 6, L(copy96)
	ldp	D_l, D_h, [srcend, -16]
	tbz	tmp1, 5, 1f
	ldp	B_l, B_h, [src, 16]
	ldp	C_l, C_h, [srcend, -32]
	stp	B_l, B_h, [dstin, 16]
	stp	C_l, C_h, [dstend, -32]
1:
	stp	A_l, A_h, [dstin]
	stp	D_l, D_h, [dstend, -16]
	ret

	.p2align 4
	/* Small copies: 0..16 bytes.  */
L(copy16):
	cmp	count, 8
	b.lo	1f
	ldr	A_l, [src]
	ldr	A_h, [srcend, -8]
	str	A_l, [dstin]
	str	A_h, [dstend, -8]
	ret
	.p2align 4
1:
	/* 4-7 */
	tbz	count, 2, 1f
	ldr	A_lw, [src]
	ldr	A_hw, [srcend, -4]
	str	A_lw, [dstin]
	str	A_hw, [dstend, -4]
	ret
	.p2align 4
1:
	/* 2-3 */
	tbz	count, 1, 1f
	ldrh	A_lw, [src]
	ldrh	A_hw, [srcend, -2]
	strh	A_lw, [dstin]
	strh	A_hw, [dstend, -2]
	ret
	.p2align 4
1:
	/* 0-1 */
	tbz	count, 0, 1f
	ldrb	A_lw, [src]
	strb	A_lw, [dstin]
1:	ret

	.p2align 4
	/* Copy 64..96 bytes.  Copy 64 bytes from the start and
	   32 bytes from the end.  */
L(copy96):
	ldp	B_l, B_h, [src, 16]
	ldp	C_l, C_h, [src, 32]
	ldp	D_l, D_h, [src, 48]
	ldp	E_l, E_h, [srcend, -32]
	ldp	F_l, F_h, [srcend, -16]
	stp	A_l, A_h, [dstin]
	stp	B_l, B_h, [dstin, 16]
	stp	C_l, C_h, [dstin, 32]
	stp	D_l, D_h, [dstin, 48]
	stp	E_l, E_h, [dstend, -32]
	stp	F_l, F_h, [dstend, -16]
	ret

	/* Align SRC to 16 byte alignment so that we don't cross cache line
	   boundaries on both loads and stores.  There are at least 96 bytes
	   to copy, so copy 16 bytes unaligned and then align.  The loop
	   copies 32 bytes per iteration and prefetches one iteration ahead.  */

	.p2align 4
L(copy_long):
	sub	count, count, 64 + 16	/* Test and readjust count.  */
	mov	B_l, Q_l
	mov	B_h, Q_h
	ldp	A_l, A_h, [src]
	and	tmp1, src, 15
	bic	src, src, 15
	sub	dst, dstin, tmp1
	add	count, count, tmp1	/* Count is now 16 too large.  */
	ldp	Q_l, Q_h, [src, 16]!
	stp	A_l, A_h, [dstin]
	ldp	A_l, A_h, [src, 16]!

L(loop64):
	subs	count, count, 32
	stp	Q_l, Q_h, [dst, 16]
	ldp	Q_l, Q_h, [src, 16]!
	stp	A_l, A_h, [dst, 32]!
	ldp	A_l, A_h, [src, 16]!
	b.hi	L(loop64)

	/* Write the last full set of 32 bytes.  The remainder is at most 32
	   bytes, so it is safe to always copy 32 bytes from the end even if
	   there is just 1 byte left.  */
L(last64):
	ldp	C_l, C_h, [srcend, -32]
	stp	Q_l, Q_h, [dst, 16]
	ldp	Q_l, Q_h, [srcend, -16]
	stp	A_l, A_h, [dst, 32]
	stp	C_l, C_h, [dstend, -32]
	stp	Q_l, Q_h, [dstend, -16]
	mov	Q_l, B_l
	mov	Q_h, B_h
	ret

	.p2align 4
L(move_long):
	cbz	tmp1, 3f

	mov	B_l, Q_l
	mov	B_h, Q_h

	/* Align SRCEND to 16 byte alignment so that we don't cross cache line
	   boundaries on both loads and stores.  There are at least 96 bytes
	   to copy, so copy 16 bytes unaligned and then align.  The loop
	   copies 32 bytes per iteration and prefetches one iteration ahead.  */

	ldp	A_l, A_h, [srcend, -16]
	and	tmp1, srcend, 15
	sub	srcend, srcend, tmp1
	ldp	Q_l, Q_h, [srcend, -16]!
	stp	A_l, A_h, [dstend, -16]
	sub	count, count, tmp1
	ldp	A_l, A_h, [srcend, -16]!
	sub	dstend, dstend, tmp1
	sub	count, count, 64

1:
	subs	count, count, 32
	stp	Q_l, Q_h, [dstend, -16]
	ldp	Q_l, Q_h, [srcend, -16]!
	stp	A_l, A_h, [dstend, -32]!
	ldp	A_l, A_h, [srcend, -16]!
	b.hi	1b

	/* Write the last full set of 32 bytes.  The remainder is at most 32
	   bytes, so it is safe to always copy 32 bytes from the start even if
	   there is just 1 byte left.  */
2:
	ldp	C_l, C_h, [src, 16]
	stp	Q_l, Q_h, [dstend, -16]
	ldp	Q_l, Q_h, [src]
	stp	A_l, A_h, [dstend, -32]
	stp	C_l, C_h, [dstin, 16]
	stp	Q_l, Q_h, [dstin]
	mov	Q_l, B_l
	mov	Q_h, B_h
3:	ret

END (__memmove_falkor)
libc_hidden_builtin_def (__memmove_falkor)
