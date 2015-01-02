/* strchr - find a character in a string

   Copyright (C) 2014-2015 Free Software Foundation, Inc.

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

/* Arguments and results.  */
#define srcin		x0
#define chrin		w1

#define result		x0

#define src		x2
#define tmp1		x3
#define wtmp2		w4
#define tmp3		x5

#define vrepchr		v0
#define vdata1		v1
#define vdata2		v2
#define vhas_nul1	v3
#define vhas_nul2	v4
#define vhas_chr1	v5
#define vhas_chr2	v6
#define vrepmask_0	v7
#define vrepmask_c	v16
#define vend1		v17
#define vend2		v18

	/* Core algorithm.
	   For each 32-byte hunk we calculate a 64-bit syndrome value, with
	   two bits per byte (LSB is always in bits 0 and 1, for both big
	   and little-endian systems).  Bit 0 is set iff the relevant byte
	   matched the requested character.  Bit 1 is set iff the
	   relevant byte matched the NUL end of string (we trigger off bit0
	   for the special case of looking for NUL).  Since the bits
	   in the syndrome reflect exactly the order in which things occur
	   in the original string a count_trailing_zeros() operation will
	   identify exactly which byte is causing the termination, and why.  */

/* Locals and temporaries.  */

ENTRY (strchr)
	mov	wtmp2, #0x0401
	movk	wtmp2, #0x4010, lsl #16
	dup	vrepchr.16b, chrin
	bic	src, srcin, #31
	dup	vrepmask_c.4s, wtmp2
	ands	tmp1, srcin, #31
	add	vrepmask_0.4s, vrepmask_c.4s, vrepmask_c.4s // lsl #1
	b.eq	L(loop)

	/* Input string is not 32-byte aligned.  Rather than forcing
	   the padding bytes to a safe value, we calculate the syndrome
	   for all the bytes, but then mask off those bits of the
	   syndrome that are related to the padding.  */
	ld1	{vdata1.16b, vdata2.16b}, [src], #32
	neg	tmp1, tmp1
	cmeq	vhas_nul1.16b, vdata1.16b, #0
	cmeq	vhas_chr1.16b, vdata1.16b, vrepchr.16b
	cmeq	vhas_nul2.16b, vdata2.16b, #0
	cmeq	vhas_chr2.16b, vdata2.16b, vrepchr.16b
	and	vhas_nul1.16b, vhas_nul1.16b, vrepmask_0.16b
	and	vhas_nul2.16b, vhas_nul2.16b, vrepmask_0.16b
	and	vhas_chr1.16b, vhas_chr1.16b, vrepmask_c.16b
	and	vhas_chr2.16b, vhas_chr2.16b, vrepmask_c.16b
	orr	vend1.16b, vhas_nul1.16b, vhas_chr1.16b
	orr	vend2.16b, vhas_nul2.16b, vhas_chr2.16b
	lsl	tmp1, tmp1, #1
	addp	vend1.16b, vend1.16b, vend2.16b		// 256->128
	mov	tmp3, #~0
	addp	vend1.16b, vend1.16b, vend2.16b		// 128->64
	lsr	tmp1, tmp3, tmp1

	mov	tmp3, vend1.2d[0]
	bic	tmp1, tmp3, tmp1	// Mask padding bits.
	cbnz	tmp1, L(tail)

L(loop):
	ld1	{vdata1.16b, vdata2.16b}, [src], #32
	cmeq	vhas_nul1.16b, vdata1.16b, #0
	cmeq	vhas_chr1.16b, vdata1.16b, vrepchr.16b
	cmeq	vhas_nul2.16b, vdata2.16b, #0
	cmeq	vhas_chr2.16b, vdata2.16b, vrepchr.16b
	/* Use a fast check for the termination condition.  */
	orr	vend1.16b, vhas_nul1.16b, vhas_chr1.16b
	orr	vend2.16b, vhas_nul2.16b, vhas_chr2.16b
	orr	vend1.16b, vend1.16b, vend2.16b
	addp	vend1.2d, vend1.2d, vend1.2d
	mov	tmp1, vend1.2d[0]
	cbz	tmp1, L(loop)

	/* Termination condition found.  Now need to establish exactly why
	   we terminated.  */
	and	vhas_nul1.16b, vhas_nul1.16b, vrepmask_0.16b
	and	vhas_nul2.16b, vhas_nul2.16b, vrepmask_0.16b
	and	vhas_chr1.16b, vhas_chr1.16b, vrepmask_c.16b
	and	vhas_chr2.16b, vhas_chr2.16b, vrepmask_c.16b
	orr	vend1.16b, vhas_nul1.16b, vhas_chr1.16b
	orr	vend2.16b, vhas_nul2.16b, vhas_chr2.16b
	addp	vend1.16b, vend1.16b, vend2.16b		// 256->128
	addp	vend1.16b, vend1.16b, vend2.16b		// 128->64

	mov	tmp1, vend1.2d[0]
L(tail):
	sub	src, src, #32
	rbit	tmp1, tmp1
	clz	tmp1, tmp1
	/* Tmp1 is even if the target charager was found first.  Otherwise
	   we've found the end of string and we weren't looking for NUL.  */
	tst	tmp1, #1
	add	result, src, tmp1, lsr #1
	csel	result, result, xzr, eq
	ret
END (strchr)
libc_hidden_builtin_def (strchr)
weak_alias (strchr, index)
