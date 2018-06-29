/* Optimized memcpy for Qualcomm Falkor processor.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.

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

   ARMv8-a, AArch64, falkor, unaligned accesses.  */

#define dstin	x0
#define src	x1
#define count	x2
#define dst	x3
#define srcend	x4
#define dstend	x5
#define tmp1	x14
#define A_x	x6
#define B_x	x7
#define A_w	w6
#define B_w	w7

#define A_q	q0
#define B_q	q1
#define C_q	q2
#define D_q	q3
#define E_q	q4
#define F_q	q5
#define G_q	q6

/* Copies are split into 3 main cases:

   1. Small copies of up to 32 bytes
   2. Medium copies of 33..128 bytes which are fully unrolled
   3. Large copies of more than 128 bytes.

   Large copies align the sourceto a quad word and use an unrolled loop
   processing 64 bytes per iteration.

   FALKOR-SPECIFIC DESIGN:

   The smallest copies (32 bytes or less) focus on optimal pipeline usage,
   which is why the redundant copies of 0-3 bytes have been replaced with
   conditionals, since the former would unnecessarily break across multiple
   issue groups.  The medium copy group has been enlarged to 128 bytes since
   bumping up the small copies up to 32 bytes allows us to do that without
   cost and also allows us to reduce the size of the prep code before loop64.

   The copy loop uses only one register q0.  This is to ensure that all loads
   hit a single hardware prefetcher which can get correctly trained to prefetch
   a single stream.

   The non-temporal stores help optimize cache utilization.  */

#if IS_IN (libc)
ENTRY_ALIGN (__memcpy_falkor, 6)

	cmp	count, 32
	add	srcend, src, count
	add	dstend, dstin, count
	b.ls	L(copy32)
	ldr	A_q, [src]
	cmp	count, 128
	str	A_q, [dstin]
	b.hi	L(copy_long)

	/* Medium copies: 33..128 bytes.  */
	sub	tmp1, count, 1
	ldr	A_q, [src, 16]
	ldr	B_q, [srcend, -32]
	ldr	C_q, [srcend, -16]
	tbz	tmp1, 6, 1f
	ldr	D_q, [src, 32]
	ldr	E_q, [src, 48]
	str	D_q, [dstin, 32]
	str	E_q, [dstin, 48]
	ldr	F_q, [srcend, -64]
	ldr	G_q, [srcend, -48]
	str	F_q, [dstend, -64]
	str	G_q, [dstend, -48]
1:
	str	A_q, [dstin, 16]
	str	B_q, [dstend, -32]
	str	C_q, [dstend, -16]
	ret

	.p2align 4
	/* Small copies: 0..32 bytes.  */
L(copy32):
	/* 16-32 */
	cmp	count, 16
	b.lo	1f
	ldr	A_q, [src]
	ldr	B_q, [srcend, -16]
	str	A_q, [dstin]
	str	B_q, [dstend, -16]
	ret
	.p2align 4
1:
	/* 8-15 */
	tbz	count, 3, 1f
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
1:
	ret

	/* Align SRC to 16 bytes and copy; that way at least one of the
	   accesses is aligned throughout the copy sequence.

	   The count is off by 0 to 15 bytes, but this is OK because we trim
	   off the last 64 bytes to copy off from the end.  Due to this the
	   loop never runs out of bounds.  */
	.p2align 6
L(copy_long):
	sub	count, count, 64 + 16
	and	tmp1, src, 15
	bic	src, src, 15
	sub	dst, dstin, tmp1
	add	count, count, tmp1

L(loop64):
	ldr	A_q, [src, 16]!
	str	A_q, [dst, 16]
	ldr	A_q, [src, 16]!
	subs	count, count, 64
	str	A_q, [dst, 32]
	ldr	A_q, [src, 16]!
	str	A_q, [dst, 48]
	ldr	A_q, [src, 16]!
	str	A_q, [dst, 64]!
	b.hi	L(loop64)

	/* Write the last full set of 64 bytes.  The remainder is at most 64
	   bytes, so it is safe to always copy 64 bytes from the end even if
	   there is just 1 byte left.  */
L(last64):
	ldr	E_q, [srcend, -64]
	str	E_q, [dstend, -64]
	ldr	D_q, [srcend, -48]
	str	D_q, [dstend, -48]
	ldr	C_q, [srcend, -32]
	str	C_q, [dstend, -32]
	ldr	B_q, [srcend, -16]
	str	B_q, [dstend, -16]
	ret

END (__memcpy_falkor)
libc_hidden_builtin_def (__memcpy_falkor)
#endif
