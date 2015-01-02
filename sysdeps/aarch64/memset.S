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

#include <sysdep.h>

/* By default we assume that the DC instruction can be used to zero
   data blocks more efficiently.  In some circumstances this might be
   unsafe, for example in an asymmetric multiprocessor environment with
   different DC clear lengths (neither the upper nor lower lengths are
   safe to use).  The feature can be disabled by defining DONT_USE_DC.

   If code may be run in a virtualized environment, then define
   MAYBE_VIRT.  This will cause the code to cache the system register
   values rather than re-reading them each call.  */

#define dstin		x0
#define val		w1
#define count		x2
#define tmp1		x3
#define tmp1w		w3
#define tmp2		x4
#define tmp2w		w4
#define zva_len_x	x5
#define zva_len		w5
#define zva_bits_x	x6

#define A_l		x7
#define A_lw		w7
#define dst		x8
#define tmp3w		w9

ENTRY_ALIGN (__memset, 6)

	mov	dst, dstin		/* Preserve return value.  */
	ands	A_lw, val, #255
#ifndef DONT_USE_DC
	b.eq	L(zero_mem)
#endif
	orr	A_lw, A_lw, A_lw, lsl #8
	orr	A_lw, A_lw, A_lw, lsl #16
	orr	A_l, A_l, A_l, lsl #32
L(tail_maybe_long):
	cmp	count, #64
	b.ge	L(not_short)
L(tail_maybe_tiny):
	cmp	count, #15
	b.le	L(tail15tiny)
L(tail63):
	ands	tmp1, count, #0x30
	b.eq	L(tail15)
	add	dst, dst, tmp1
	cmp	tmp1w, #0x20
	b.eq	1f
	b.lt	2f
	stp	A_l, A_l, [dst, #-48]
1:
	stp	A_l, A_l, [dst, #-32]
2:
	stp	A_l, A_l, [dst, #-16]

L(tail15):
	and	count, count, #15
	add	dst, dst, count
	stp	A_l, A_l, [dst, #-16]	/* Repeat some/all of last store. */
	RET

L(tail15tiny):
	/* Set up to 15 bytes.  Does not assume earlier memory
	   being set.  */
	tbz	count, #3, 1f
	str	A_l, [dst], #8
1:
	tbz	count, #2, 1f
	str	A_lw, [dst], #4
1:
	tbz	count, #1, 1f
	strh	A_lw, [dst], #2
1:
	tbz	count, #0, 1f
	strb	A_lw, [dst]
1:
	RET

	/* Critical loop.  Start at a new cache line boundary.  Assuming
	 * 64 bytes per line, this ensures the entire loop is in one line.  */
	.p2align 6
L(not_short):
	neg	tmp2, dst
	ands	tmp2, tmp2, #15
	b.eq	2f
	/* Bring DST to 128-bit (16-byte) alignment.  We know that there's
	 * more than that to set, so we simply store 16 bytes and advance by
	 * the amount required to reach alignment.  */
	sub	count, count, tmp2
	stp	A_l, A_l, [dst]
	add	dst, dst, tmp2
	/* There may be less than 63 bytes to go now.  */
	cmp	count, #63
	b.le	L(tail63)
2:
	sub	dst, dst, #16		/* Pre-bias.  */
	sub	count, count, #64
1:
	stp	A_l, A_l, [dst, #16]
	stp	A_l, A_l, [dst, #32]
	stp	A_l, A_l, [dst, #48]
	stp	A_l, A_l, [dst, #64]!
	subs	count, count, #64
	b.ge	1b
	tst	count, #0x3f
	add	dst, dst, #16
	b.ne	L(tail63)
	RET

#ifndef DONT_USE_DC
	/* For zeroing memory, check to see if we can use the ZVA feature to
	 * zero entire 'cache' lines.  */
L(zero_mem):
	mov	A_l, #0
	cmp	count, #63
	b.le	L(tail_maybe_tiny)
	neg	tmp2, dst
	ands	tmp2, tmp2, #15
	b.eq	1f
	sub	count, count, tmp2
	stp	A_l, A_l, [dst]
	add	dst, dst, tmp2
	cmp	count, #63
	b.le	L(tail63)
1:
	/* For zeroing small amounts of memory, it's not worth setting up
	 * the line-clear code.  */
	cmp	count, #128
	b.lt	L(not_short)
#ifdef MAYBE_VIRT
	/* For efficiency when virtualized, we cache the ZVA capability.  */
	adrp	tmp2, L(cache_clear)
	ldr	zva_len, [tmp2, #:lo12:L(cache_clear)]
	tbnz	zva_len, #31, L(not_short)
	cbnz	zva_len, L(zero_by_line)
	mrs	tmp1, dczid_el0
	tbz	tmp1, #4, 1f
	/* ZVA not available.  Remember this for next time.  */
	mov	zva_len, #~0
	str	zva_len, [tmp2, #:lo12:L(cache_clear)]
	b	L(not_short)
1:
	mov	tmp3w, #4
	and	zva_len, tmp1w, #15	/* Safety: other bits reserved.  */
	lsl	zva_len, tmp3w, zva_len
	str	zva_len, [tmp2, #:lo12:L(cache_clear)]
#else
	mrs	tmp1, dczid_el0
	tbnz	tmp1, #4, L(not_short)
	mov	tmp3w, #4
	and	zva_len, tmp1w, #15	/* Safety: other bits reserved.  */
	lsl	zva_len, tmp3w, zva_len
#endif

L(zero_by_line):
	/* Compute how far we need to go to become suitably aligned.  We're
	 * already at quad-word alignment.  */
	cmp	count, zva_len_x
	b.lt	L(not_short)		/* Not enough to reach alignment.  */
	sub	zva_bits_x, zva_len_x, #1
	neg	tmp2, dst
	ands	tmp2, tmp2, zva_bits_x
	b.eq	1f			/* Already aligned.  */
	/* Not aligned, check that there's enough to copy after alignment.  */
	sub	tmp1, count, tmp2
	cmp	tmp1, #64
	ccmp	tmp1, zva_len_x, #8, ge	/* NZCV=0b1000 */
	b.lt	L(not_short)
	/* We know that there's at least 64 bytes to zero and that it's safe
	 * to overrun by 64 bytes.  */
	mov	count, tmp1
2:
	stp	A_l, A_l, [dst]
	stp	A_l, A_l, [dst, #16]
	stp	A_l, A_l, [dst, #32]
	subs	tmp2, tmp2, #64
	stp	A_l, A_l, [dst, #48]
	add	dst, dst, #64
	b.ge	2b
	/* We've overrun a bit, so adjust dst downwards.  */
	add	dst, dst, tmp2
1:
	sub	count, count, zva_len_x
3:
	dc	zva, dst
	add	dst, dst, zva_len_x
	subs	count, count, zva_len_x
	b.ge	3b
	ands	count, count, zva_bits_x
	b.ne	L(tail_maybe_long)
	RET
#ifdef MAYBE_VIRT
	.bss
	.p2align 2
L(cache_clear):
	.space 4
#endif
#endif /* DONT_USE_DC */

END (__memset)
weak_alias (__memset, memset)
libc_hidden_builtin_def (memset)
