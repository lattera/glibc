/* Optimized strnlen implementation for PowerPC32/POWER7 using cmpb insn.
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

/* int [r3] strnlen (char *s [r3], int size [r4])  */
	.machine  power7
ENTRY (BP_SYM (__strnlen))
	CALL_MCOUNT
	dcbt	0,r3
	clrrwi	r8,r3,2	      /* Align the address to word boundary.  */
	add	r7,r3,r4      /* Calculate the last acceptable address.  */
	cmplwi	r4,16
	li	r0,0	      /* Word with null chars.  */
	ble	L(small_range)

	cmplw	cr7,r3,r7     /* Is the address equal or less than r3?  If
				 it's equal or less, it means size is either 0
				 or a negative number.  */
	ble	cr7,L(proceed)

	li	r7,-1	      /* Make r11 the biggest if r4 <= 0.  */
L(proceed):
	rlwinm	r6,r3,3,27,28 /* Calculate padding.  */
	lwz	r12,0(r8)     /* Load word from memory.  */
	cmpb	r10,r12,r0    /* Check for null bytes in DWORD1.  */
	slw	r10,r10,r6
	srw	r10,r10,r6
	cmplwi	cr7,r10,0     /* If r10 == 0, no null's have been found.  */
	bne	cr7,L(done)

	/* Are we done already?  */
	addi	r9,r8,4
	cmplw	cr6,r9,r7
	bge	cr6,L(end_max)

	mtcrf   0x01,r8
	/* Are we now aligned to a doubleword boundary?  If so, skip to
	   the main loop.  Otherwise, go through the alignment code.  */

	bt	29,L(loop_setup)

	/* Handle DWORD2 of pair.  */
	lwzu	r12,4(r8)
	cmpb	r10,r12,r0
	cmplwi	cr7,r10,0
	bne	cr7,L(done)

	/* Are we done already?  */
	addi	r9,r8,4
	cmplw	cr6,r9,r7
	bge	cr6,L(end_max)

L(loop_setup):
	sub	r5,r7,r9
	srwi	r6,r5,3	      /* Number of loop iterations.  */
	mtctr	r6	      /* Setup the counter.  */
	b	L(loop)
	/* Main loop to look for the null byte backwards in the string.  Since
	   it's a small loop (< 8 instructions), align it to 32-bytes.  */
	.p2align  5
L(loop):
	/* Load two words, compare and merge in a
	   single register for speed.  This is an attempt
	   to speed up the null-checking process for bigger strings.  */

	lwz	r12,4(r8)
	lwzu	r11,8(r8)
	cmpb	r10,r12,r0
	cmpb	r9,r11,r0
	or	r5,r9,r10     /* Merge everything in one word.  */
	cmplwi	cr7,r5,0
	bne	cr7,L(found)
	bdnz	L(loop)
	/* We're here because the counter reached 0, and that means we
	   didn't have any matches for null in the whole range.  Just return
	   the original size.  */
	addi	r9,r8,4
	cmplw	cr6,r9,r7
	blt	cr6,L(loop_small)

L(end_max):
	sub	r3,r7,r3
	blr

	/* OK, one (or both) of the words contains a null byte.  Check
	   the first word and decrement the address in case the first
	   word really contains a null byte.  */
	.align	4
L(found):
	cmplwi	cr6,r10,0
	addi	r8,r8,-4
	bne	cr6,L(done)

	/* The null byte must be in the second word.  Adjust the address
	   again and move the result of cmpb to r10 so we can calculate the
	   length.  */

	mr	r10,r9
	addi	r8,r8,4

	/* r10 has the output of the cmpb instruction, that is, it contains
	   0xff in the same position as the null byte in the original
	   word from the string.  Use that to calculate the length.
	   We need to make sure the null char is *before* the end of the
	   range.  */
L(done):
	cntlzw	r0,r10	      /* Count leading zeroes before the match.  */
	srwi	r0,r0,3	      /* Convert leading zeroes to bytes.  */
	add	r9,r8,r0
	sub	r6,r9,r3      /* Length until the match.  */
	cmplw	r9,r7
	bgt	L(end_max)
	mr	r3,r6
	blr

	.align	4
L(zero):
	li	r3,0
	blr

/* Deals with size <= 32.  */
	.align	4
L(small_range):
	cmplwi	r4,0
	beq	L(zero)

	rlwinm	r6,r3,3,27,28 /* Calculate padding.  */
	lwz	r12,0(r8)     /* Load word from memory.  */
	cmpb	r10,r12,r0    /* Check for null bytes in WORD1.  */
	slw	r10,r10,r6
	srw	r10,r10,r6
	cmplwi	cr7,r10,0
	bne	cr7,L(done)

	addi    r9,r8,4
	cmplw	r9,r7
	bge	L(end_max)
	b	L(loop_small)

	.p2align  5
L(loop_small):
	lwzu	r12,4(r8)
	cmpb	r10,r12,r0
	addi	r9,r8,4
	cmplwi	cr6,r10,0
	bne	cr6,L(done)
	cmplw	r9,r7
	bge	L(end_max)
	b	L(loop_small)
END (BP_SYM (__strnlen))
weak_alias (BP_SYM (__strnlen), BP_SYM(strnlen))
libc_hidden_builtin_def (strnlen)
