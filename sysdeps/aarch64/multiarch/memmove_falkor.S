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
#define dst	x3
#define srcend	x4
#define dstend	x5
#define A_x	x6
#define B_x	x7
#define A_w	w6
#define B_w	w7
#define tmp1	x14

#define Q_q	q6
#define A_q	q22
#define B_q	q18
#define C_q	q19
#define D_q	q20
#define E_q	q21
#define F_q	q17
#define G_q	q23

/* RATIONALE:

   The move has 4 distinct parts:
   * Small moves of 16 bytes and under
   * Medium sized moves of 17-96 bytes
   * Large moves where the source address is higher than the destination
     (forward copies)
   * Large moves where the destination address is higher than the source
     (copy backward, or move).

   We use only two registers q6 and q22 for the moves and move 32 bytes at a
   time to correctly train the hardware prefetcher for better throughput.  */
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
	ldr	A_q, [src]
	tbnz	tmp1, 6, L(copy96)
	ldr	D_q, [srcend, -16]
	tbz	tmp1, 5, 1f
	ldr	B_q, [src, 16]
	ldr	C_q, [srcend, -32]
	str	B_q, [dstin, 16]
	str	C_q, [dstend, -32]
1:
	str	A_q, [dstin]
	str	D_q, [dstend, -16]
	ret

	.p2align 4
	/* Small copies: 0..16 bytes.  */
L(copy16):
	cmp	count, 8
	b.lo	1f
	ldr	A_x, [src]
	ldr	B_x, [srcend, -8]
	str	A_x, [dstin]
	str	B_x, [dstend, -8]
	ret
	.p2align 4
1:
	/* 4-7 */
	tbz	count, 2, 1f
	ldr	A_w, [src]
	ldr	B_w, [srcend, -4]
	str	A_w, [dstin]
	str	B_w, [dstend, -4]
	ret
	.p2align 4
1:
	/* 2-3 */
	tbz	count, 1, 1f
	ldrh	A_w, [src]
	ldrh	B_w, [srcend, -2]
	strh	A_w, [dstin]
	strh	B_w, [dstend, -2]
	ret
	.p2align 4
1:
	/* 0-1 */
	tbz	count, 0, 1f
	ldrb	A_w, [src]
	strb	A_w, [dstin]
1:	ret

	.p2align 4
	/* Copy 64..96 bytes.  Copy 64 bytes from the start and
	   32 bytes from the end.  */
L(copy96):
	ldr	B_q, [src, 16]
	ldr	C_q, [src, 32]
	ldr	D_q, [src, 48]
	ldr	E_q, [srcend, -32]
	ldr	F_q, [srcend, -16]
	str	A_q, [dstin]
	str	B_q, [dstin, 16]
	str	C_q, [dstin, 32]
	str	D_q, [dstin, 48]
	str	E_q, [dstend, -32]
	str	F_q, [dstend, -16]
	ret

	/* Align SRC to 16 byte alignment so that we don't cross cache line
	   boundaries on both loads and stores.  There are at least 96 bytes
	   to copy, so copy 16 bytes unaligned and then align.  The loop
	   copies 32 bytes per iteration and prefetches one iteration ahead.  */

	.p2align 4
L(copy_long):
	ldr	A_q, [src]
	and	tmp1, src, 15
	bic	src, src, 15
	sub	dst, dstin, tmp1
	add	count, count, tmp1	/* Count is now 16 too large.  */
	ldr	Q_q, [src, 16]!
	str	A_q, [dstin]
	ldr	A_q, [src, 16]!
	subs	count, count, 32 + 64 + 16	/* Test and readjust count.  */
	b.ls	L(last64)

L(loop64):
	subs	count, count, 32
	str	Q_q, [dst, 16]
	ldr	Q_q, [src, 16]!
	str	A_q, [dst, 32]!
	ldr	A_q, [src, 16]!
	b.hi	L(loop64)

	/* Write the last full set of 64 bytes.  The remainder is at most 64
	   bytes and at least 33 bytes, so it is safe to always copy 64 bytes
	   from the end.  */
L(last64):
	ldr	C_q, [srcend, -64]
	str	Q_q, [dst, 16]
	ldr	B_q, [srcend, -48]
	str	A_q, [dst, 32]
	ldr	A_q, [srcend, -32]
	ldr	D_q, [srcend, -16]
	str	C_q, [dstend, -64]
	str	B_q, [dstend, -48]
	str	A_q, [dstend, -32]
	str	D_q, [dstend, -16]
	ret

	.p2align 4
L(move_long):
	cbz	tmp1, 3f

	/* Align SRCEND to 16 byte alignment so that we don't cross cache line
	   boundaries on both loads and stores.  There are at least 96 bytes
	   to copy, so copy 16 bytes unaligned and then align.  The loop
	   copies 32 bytes per iteration and prefetches one iteration ahead.  */

	ldr	A_q, [srcend, -16]
	and	tmp1, srcend, 15
	sub	srcend, srcend, tmp1
	ldr	Q_q, [srcend, -16]!
	str	A_q, [dstend, -16]
	sub	count, count, tmp1
	ldr	A_q, [srcend, -16]!
	sub	dstend, dstend, tmp1
	subs	count, count, 32 + 64
	b.ls	2f

1:
	subs	count, count, 32
	str	Q_q, [dstend, -16]
	ldr	Q_q, [srcend, -16]!
	str	A_q, [dstend, -32]!
	ldr	A_q, [srcend, -16]!
	b.hi	1b

	/* Write the last full set of 64 bytes.  The remainder is at most 64
	   bytes and at least 33 bytes, so it is safe to always copy 64 bytes
	   from the start.  */
2:
	ldr	C_q, [src, 48]
	str	Q_q, [dstend, -16]
	ldr	B_q, [src, 32]
	str	A_q, [dstend, -32]
	ldr	A_q, [src, 16]
	ldr	D_q, [src]
	str	C_q, [dstin, 48]
	str	B_q, [dstin, 32]
	str	A_q, [dstin, 16]
	str	D_q, [dstin]
3:	ret

END (__memmove_falkor)
libc_hidden_builtin_def (__memmove_falkor)
