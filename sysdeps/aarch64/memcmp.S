/* memcmp - compare memory

   Copyright (C) 2013-2018 Free Software Foundation, Inc.

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
 * ARMv8-a, AArch64, unaligned accesses.
 */

/* Parameters and result.  */
#define src1		x0
#define src2		x1
#define limit		x2
#define result		w0

/* Internal variables.  */
#define data1		x3
#define data1w		w3
#define data2		x4
#define data2w		w4
#define tmp1		x5

ENTRY_ALIGN (memcmp, 6)
	DELOUSE (0)
	DELOUSE (1)
	DELOUSE (2)

	subs	limit, limit, 8
	b.lo	.Lless8

	/* Limit >= 8, so check first 8 bytes using unaligned loads.  */
	ldr	data1, [src1], 8
	ldr	data2, [src2], 8
	and	tmp1, src1, 7
	add	limit, limit, tmp1
	cmp	data1, data2
	bne	.Lreturn

	/* Align src1 and adjust src2 with bytes not yet done.  */
	sub	src1, src1, tmp1
	sub	src2, src2, tmp1

	subs	limit, limit, 8
	b.ls	.Llast_bytes

	/* Loop performing 8 bytes per iteration using aligned src1.
	   Limit is pre-decremented by 8 and must be larger than zero.
	   Exit if <= 8 bytes left to do or if the data is not equal.  */
	.p2align 4
.Lloop8:
	ldr	data1, [src1], 8
	ldr	data2, [src2], 8
	subs	limit, limit, 8
	ccmp	data1, data2, 0, hi  /* NZCV = 0b0000.  */
	b.eq	.Lloop8

	cmp	data1, data2
	bne	.Lreturn

	/* Compare last 1-8 bytes using unaligned access.  */
.Llast_bytes:
	ldr	data1, [src1, limit]
	ldr	data2, [src2, limit]

	/* Compare data bytes and set return value to 0, -1 or 1.  */
.Lreturn:
#ifndef __AARCH64EB__
	rev	data1, data1
	rev	data2, data2
#endif
	cmp     data1, data2
.Lret_eq:
	cset	result, ne
	cneg	result, result, lo
	ret

	.p2align 4
	/* Compare up to 8 bytes.  Limit is [-8..-1].  */
.Lless8:
	adds	limit, limit, 4
	b.lo	.Lless4
	ldr	data1w, [src1], 4
	ldr	data2w, [src2], 4
	cmp	data1w, data2w
	b.ne	.Lreturn
	sub	limit, limit, 4
.Lless4:
	adds	limit, limit, 4
	beq	.Lret_eq
.Lbyte_loop:
	ldrb	data1w, [src1], 1
	ldrb	data2w, [src2], 1
	subs	limit, limit, 1
	ccmp	data1w, data2w, 0, ne	/* NZCV = 0b0000.  */
	b.eq	.Lbyte_loop
	sub	result, data1w, data2w
	ret

END (memcmp)
#undef bcmp
weak_alias (memcmp, bcmp)
libc_hidden_builtin_def (memcmp)
