/* Optimized memrchr implementation for PowerPC32/POWER7 using cmpb insn.
   Copyright (C) 2010 Free Software Foundation, Inc.
   Contributed by Luis Machado <luisgpm@br.ibm.com>.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA
   02110-1301 USA.  */

#include <sysdep.h>
#include <bp-sym.h>
#include <bp-asm.h>

/* int [r3] memrchr (char *s [r3], int byte [r4], int size [r5])  */
	.machine  power7
ENTRY (BP_SYM (__memrchr))
	CALL_MCOUNT
	dcbt	0,r3
	mr	r7,r3
	add	r3,r7,r5      /* Calculate the last acceptable address.  */
	cmplw	cr7,r3,r7     /* Is the address equal or less than r3?  */

	/* Replicate BYTE to word.  */
	rlwimi	r4,r4,8,16,23
	rlwimi	r4,r4,16,0,15
	bge	cr7,L(proceed)

	li	r3,-1	      /* Make r11 the biggest if r4 <= 0.  */
L(proceed):
	li	r6,-4
	addi	r9,r3,-1
	clrrwi  r8,r9,2
	addi	r8,r8,4
	neg	r0,r3
	rlwinm	r0,r0,3,27,28 /* Calculate padding.  */

	cmplwi	r5,16
	ble	L(small_range)

	lwbrx	r12,r8,r6     /* Load reversed word from memory.  */
	cmpb	r10,r12,r4    /* Check for BYTE in WORD1.  */
	slw	r10,r10,r0
	srw	r10,r10,r0
	cmplwi	cr7,r10,0     /* If r10 == 0, no BYTE's have been found.  */
	bne	cr7,L(done)

	/* Are we done already?  */
	addi	r9,r8,-4
	cmplw	cr6,r9,r7
	ble	cr6,L(null)

	mtcrf   0x01,r8
	/* Are we now aligned to a doubleword boundary?  If so, skip to
	   the main loop.  Otherwise, go through the alignment code.  */
	mr	r8,r9
	bt	29,L(loop_setup)

	/* Handle WORD2 of pair.  */
	lwbrx	r12,r8,r6
	cmpb	r10,r12,r4
	cmplwi	cr7,r10,0
	bne	cr7,L(done)

	/* Are we done already?  */
	addi	r8,r8,-4
	cmplw	cr6,r8,r7
	ble	cr6,L(null)

L(loop_setup):
	li	r0,-8
	sub	r5,r8,r7
	srwi	r9,r5,3	      /* Number of loop iterations.  */
	mtctr	r9	      /* Setup the counter.  */
	b	L(loop)
	/* Main loop to look for BYTE backwards in the string.  Since it's a
	   small loop (< 8 instructions), align it to 32-bytes.  */
	.p2align  5
L(loop):
	/* Load two words, compare and merge in a
	   single register for speed.  This is an attempt
	   to speed up the byte-checking process for bigger strings.  */

	lwbrx	r12,r8,r6
	lwbrx	r11,r8,r0
	addi	r8,r8,-4
	cmpb	r10,r12,r4
	cmpb	r9,r11,r4
	or	r5,r9,r10     /* Merge everything in one word.  */
	cmplwi	cr7,r5,0
	bne	cr7,L(found)
	addi	r8,r8,-4
	bdnz	L(loop)
	/* We're here because the counter reached 0, and that means we
	   didn't have any matches for BYTE in the whole range.  Just return
	   the original range.  */
	addi	r9,r8,4
	cmplw	cr6,r9,r7
	bgt	cr6,L(loop_small)
	b	L(null)

	/* OK, one (or both) of the words contains BYTE.  Check
	   the first word and decrement the address in case the first
	   word really contains BYTE.  */
	.align	4
L(found):
	cmplwi	cr6,r10,0
	addi	r8,r8,4
	bne	cr6,L(done)

	/* BYTE must be in the second word.  Adjust the address
	   again and move the result of cmpb to r10 so we can calculate the
	   pointer.  */

	mr	r10,r9
	addi	r8,r8,-4

	/* r10 has the output of the cmpb instruction, that is, it contains
	   0xff in the same position as BYTE in the original
	   word from the string.  Use that to calculate the pointer.
	   We need to make sure BYTE is *before* the end of the
	   range.  */
L(done):
	cntlzw	r0,r10	      /* Count leading zeroes before the match.  */
	srwi	r6,r0,3	      /* Convert leading zeroes to bytes.  */
	addi	r0,r6,1
	sub	r3,r8,r0
	cmplw	r3,r7
	blt	L(null)
	blr

	.align	4
L(null):
	li	r3,0
	blr

/* Deals with size <= 16.  */
	.align	4
L(small_range):
	cmplwi	r5,0
	beq	L(null)

	lwbrx	r12,r8,r6     /* Load reversed word from memory.  */
	cmpb	r10,r12,r4    /* Check for null bytes in WORD1.  */
	slw	r10,r10,r0
	srw	r10,r10,r0
	cmplwi	cr7,r10,0
	bne	cr7,L(done)

	addi	r8,r8,-4
	cmplw	r8,r7
	ble	L(null)
	b	L(loop_small)

	.p2align  5
L(loop_small):
	lwbrx	r12,r8,r6
	cmpb	r10,r12,r4
	cmplwi	cr6,r10,0
	bne	cr6,L(done)
	addi	r8,r8,-4
	cmplw	r8,r7
	ble	L(null)
	b	L(loop_small)

END (BP_SYM (__memrchr))
weak_alias (BP_SYM (__memrchr), BP_SYM(memrchr))
libc_hidden_builtin_def (memrchr)
