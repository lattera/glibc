/* Copyright (C) 2012-2016 Free Software Foundation, Inc.

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
 */

#include <sysdep.h>

#define REP8_01 0x0101010101010101
#define REP8_7f 0x7f7f7f7f7f7f7f7f
#define REP8_80 0x8080808080808080

/* Parameters and result.  */
#define src1		x0
#define src2		x1
#define result		x0

/* Internal variables.  */
#define data1		x2
#define data1w		w2
#define data2		x3
#define data2w		w3
#define has_nul		x4
#define diff		x5
#define syndrome	x6
#define tmp1		x7
#define tmp2		x8
#define tmp3		x9
#define zeroones	x10
#define pos		x11

	/* Start of performance-critical section  -- one 64B cache line.  */
ENTRY_ALIGN(strcmp, 6)

	DELOUSE (0)
	DELOUSE (1)
	eor	tmp1, src1, src2
	mov	zeroones, #REP8_01
	tst	tmp1, #7
	b.ne	L(misaligned8)
	ands	tmp1, src1, #7
	b.ne	L(mutual_align)
	/* NUL detection works on the principle that (X - 1) & (~X) & 0x80
	   (=> (X - 1) & ~(X | 0x7f)) is non-zero iff a byte is zero, and
	   can be done in parallel across the entire word.  */
L(loop_aligned):
	ldr	data1, [src1], #8
	ldr	data2, [src2], #8
L(start_realigned):
	sub	tmp1, data1, zeroones
	orr	tmp2, data1, #REP8_7f
	eor	diff, data1, data2	/* Non-zero if differences found.  */
	bic	has_nul, tmp1, tmp2	/* Non-zero if NUL terminator.  */
	orr	syndrome, diff, has_nul
	cbz	syndrome, L(loop_aligned)
	/* End of performance-critical section  -- one 64B cache line.  */

#ifndef	__AARCH64EB__
	rev	syndrome, syndrome
	rev	data1, data1
	/* The MS-non-zero bit of the syndrome marks either the first bit
	   that is different, or the top bit of the first zero byte.
	   Shifting left now will bring the critical information into the
	   top bits.  */
	clz	pos, syndrome
	rev	data2, data2
	lsl	data1, data1, pos
	lsl	data2, data2, pos
	/* But we need to zero-extend (char is unsigned) the value and then
	   perform a signed 32-bit subtraction.  */
	lsr	data1, data1, #56
	sub	result, data1, data2, lsr #56
	RET
#else
	/* For big-endian we cannot use the trick with the syndrome value
	   as carry-propagation can corrupt the upper bits if the trailing
	   bytes in the string contain 0x01.  */
	/* However, if there is no NUL byte in the dword, we can generate
	   the result directly.  We can't just subtract the bytes as the
	   MSB might be significant.  */
	cbnz	has_nul, 1f
	cmp	data1, data2
	cset	result, ne
	cneg	result, result, lo
	RET
1:
	/* Re-compute the NUL-byte detection, using a byte-reversed value.  */
	rev	tmp3, data1
	sub	tmp1, tmp3, zeroones
	orr	tmp2, tmp3, #REP8_7f
	bic	has_nul, tmp1, tmp2
	rev	has_nul, has_nul
	orr	syndrome, diff, has_nul
	clz	pos, syndrome
	/* The MS-non-zero bit of the syndrome marks either the first bit
	   that is different, or the top bit of the first zero byte.
	   Shifting left now will bring the critical information into the
	   top bits.  */
	lsl	data1, data1, pos
	lsl	data2, data2, pos
	/* But we need to zero-extend (char is unsigned) the value and then
	   perform a signed 32-bit subtraction.  */
	lsr	data1, data1, #56
	sub	result, data1, data2, lsr #56
	RET
#endif

L(mutual_align):
	/* Sources are mutually aligned, but are not currently at an
	   alignment boundary.  Round down the addresses and then mask off
	   the bytes that preceed the start point.  */
	bic	src1, src1, #7
	bic	src2, src2, #7
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
	orr	data1, data1, tmp2
	orr	data2, data2, tmp2
	b	L(start_realigned)

L(misaligned8):
	/* We can do better than this.  */
	ldrb	data1w, [src1], #1
	ldrb	data2w, [src2], #1
	cmp	data1w, #1
	ccmp	data1w, data2w, #0, cs	/* NZCV = 0b0000.  */
	b.eq	L(misaligned8)
	sub	result, data1, data2
	RET
END(strcmp)
libc_hidden_builtin_def (strcmp)
