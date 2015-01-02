/* memcmp - compare memory

   Copyright (C) 2013-2015 Free Software Foundation, Inc.

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
 */

/* Parameters and result.  */
#define src1		x0
#define src2		x1
#define limit		x2
#define result		x0

/* Internal variables.  */
#define data1		x3
#define data1w		w3
#define data2		x4
#define data2w		w4
#define has_nul		x5
#define diff		x6
#define endloop		x7
#define tmp1		x8
#define tmp2		x9
#define tmp3		x10
#define pos		x11
#define limit_wd	x12
#define mask		x13

ENTRY_ALIGN (memcmp, 6)
	cbz	limit, L(ret0)
	eor	tmp1, src1, src2
	tst	tmp1, #7
	b.ne	L(misaligned8)
	ands	tmp1, src1, #7
	b.ne	L(mutual_align)
	add	limit_wd, limit, #7
	lsr	limit_wd, limit_wd, #3
	/* Start of performance-critical section  -- one 64B cache line.  */
L(loop_aligned):
	ldr	data1, [src1], #8
	ldr	data2, [src2], #8
L(start_realigned):
	subs	limit_wd, limit_wd, #1
	eor	diff, data1, data2	/* Non-zero if differences found.  */
	csinv	endloop, diff, xzr, ne	/* Last Dword or differences.  */
	cbz	endloop, L(loop_aligned)
	/* End of performance-critical section  -- one 64B cache line.  */

	/* Not reached the limit, must have found a diff.  */
	cbnz	limit_wd, L(not_limit)

	/* Limit % 8 == 0 => all bytes significant.  */
	ands	limit, limit, #7
	b.eq	L(not_limit)

	lsl	limit, limit, #3	/* Bits -> bytes.  */
	mov	mask, #~0
#ifdef __AARCH64EB__
	lsr	mask, mask, limit
#else
	lsl	mask, mask, limit
#endif
	bic	data1, data1, mask
	bic	data2, data2, mask

	orr	diff, diff, mask
L(not_limit):

#ifndef	__AARCH64EB__
	rev	diff, diff
	rev	data1, data1
	rev	data2, data2
#endif
	/* The MS-non-zero bit of DIFF marks either the first bit
	   that is different, or the end of the significant data.
	   Shifting left now will bring the critical information into the
	   top bits.  */
	clz	pos, diff
	lsl	data1, data1, pos
	lsl	data2, data2, pos
	/* But we need to zero-extend (char is unsigned) the value and then
	   perform a signed 32-bit subtraction.  */
	lsr	data1, data1, #56
	sub	result, data1, data2, lsr #56
	RET

L(mutual_align):
	/* Sources are mutually aligned, but are not currently at an
	   alignment boundary.  Round down the addresses and then mask off
	   the bytes that precede the start point.  */
	bic	src1, src1, #7
	bic	src2, src2, #7
	add	limit, limit, tmp1	/* Adjust the limit for the extra.  */
	lsl	tmp1, tmp1, #3		/* Bytes beyond alignment -> bits.  */
	ldr	data1, [src1], #8
	neg	tmp1, tmp1		/* Bits to alignment -64.  */
	ldr	data2, [src2], #8
	mov	tmp2, #~0
#ifdef __AARCH64EB__
	/* Big-endian.  Early bytes are at MSB.  */
	lsl	tmp2, tmp2, tmp1	/* Shift (tmp1 & 63).  */
#else
	/* Little-endian.  Early bytes are at LSB.  */
	lsr	tmp2, tmp2, tmp1	/* Shift (tmp1 & 63).  */
#endif
	add	limit_wd, limit, #7
	orr	data1, data1, tmp2
	orr	data2, data2, tmp2
	lsr	limit_wd, limit_wd, #3
	b	L(start_realigned)

L(ret0):
	mov	result, #0
	RET

	.p2align 6
L(misaligned8):
	sub	limit, limit, #1
1:
	/* Perhaps we can do better than this.  */
	ldrb	data1w, [src1], #1
	ldrb	data2w, [src2], #1
	subs	limit, limit, #1
	ccmp	data1w, data2w, #0, cs	/* NZCV = 0b0000.  */
	b.eq	1b
	sub	result, data1, data2
	RET
END (memcmp)
#undef bcmp
weak_alias (memcmp, bcmp)
libc_hidden_builtin_def (memcmp)
