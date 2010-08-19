/* Optimized strchr implementation for PowerPC64/POWER7 using cmpb insn.
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

/* int [r3] strchr (char *s [r3], int c [r4])  */
	.machine  power7
ENTRY (BP_SYM(strchr))
	CALL_MCOUNT 2
	dcbt	0,r3
	clrrdi	r8,r3,3	      /* Align the address to doubleword boundary.  */
	cmpdi	cr7,r4,0
	ld	r12,0(r8)     /* Load doubleword from memory.  */
	li	r0,0	      /* Doubleword with null chars to use
				 with cmpb.  */

	rlwinm	r6,r3,3,26,28 /* Calculate padding.  */

	beq	cr7,L(null_match)

	/* Replicate byte to doubleword.  */
	rlwimi	r4,r4,8,16,23
	rlwimi	r4,r4,16,0,15
	insrdi  r4,r4,32,0

	/* Now r4 has a doubleword of c bytes and r0 has
	   a doubleword of null bytes.  */

	cmpb	r10,r12,r4     /* Compare each byte against c byte.  */
	cmpb	r11,r12,r0     /* Compare each byte against null byte.  */

	/* Move the doublewords left and right to discard the bits that are
	   not part of the string and bring them back as zeros.  */

	sld	r10,r10,r6
	sld	r11,r11,r6
	srd	r10,r10,r6
	srd	r11,r11,r6
	or	r5,r10,r11    /* OR the results to speed things up.  */
	cmpdi	cr7,r5,0      /* If r5 == 0, no c or null bytes
				 have been found.  */
	bne	cr7,L(done)

	mtcrf   0x01,r8

	/* Are we now aligned to a doubleword boundary?  If so, skip to
	   the main loop.  Otherwise, go through the alignment code.  */

	bt	28,L(loop)

	/* Handle WORD2 of pair.  */
	ldu	r12,8(r8)
	cmpb    r10,r12,r4
	cmpb	r11,r12,r0
	or	r5,r10,r11
	cmpdi	cr7,r5,0
	bne	cr7,L(done)
	b	L(loop)	      /* We branch here (rather than falling through)
				 to skip the nops due to heavy alignment
				 of the loop below.  */

	.p2align  5
L(loop):
	/* Load two doublewords, compare and merge in a
	   single register for speed.  This is an attempt
	   to speed up the null-checking process for bigger strings.  */
	ld	r12,8(r8)
	ldu	r9,16(r8)
	cmpb	r10,r12,r4
	cmpb	r11,r12,r0
	cmpb	r6,r9,r4
	cmpb	r7,r9,r0
	or	r12,r10,r11
	or	r9,r6,r7
	or	r5,r12,r9
	cmpdi	cr7,r5,0
	beq	cr7,L(loop)

	/* OK, one (or both) of the doublewords contains a c/null byte.  Check
	   the first doubleword and decrement the address in case the first
	   doubleword really contains a c/null byte.  */

	cmpdi	cr6,r12,0
	addi	r8,r8,-8
	bne	cr6,L(done)

	/* The c/null byte must be in the second doubleword.  Adjust the
	   address again and move the result of cmpb to r10 so we can calculate
	   the pointer.  */

	mr	r10,r6
	mr	r11,r7
	addi	r8,r8,8

	/* r5 has the output of the cmpb instruction, that is, it contains
	   0xff in the same position as the c/null byte in the original
	   doubleword from the string.  Use that to calculate the pointer.  */
L(done):
	cntlzd	r4,r10	      /* Count leading zeroes before c matches.  */
	cntlzd	r0,r11	      /* Count leading zeroes before null matches.  */
	cmpld	cr7,r4,r0
	bgt	cr7,L(no_match)
	srdi	r0,r4,3	      /* Convert leading zeroes to bytes.  */
	add	r3,r8,r0      /* Return address of the matching c byte
				 or null in case c was not found.  */
	blr

	.align	4
L(no_match):
	li	r3,0
	blr

/* We are here because strchr was called with a null byte.  */
	.align	4
L(null_match):
	/* r0 has a doubleword of null bytes.  */

	cmpb	r5,r12,r0     /* Compare each byte against null bytes.  */

	/* Move the doublewords left and right to discard the bits that are
	   not part of the string and bring them back as zeros.  */

	sld	r5,r5,r6
	srd	r5,r5,r6
	cmpdi	cr7,r5,0      /* If r10 == 0, no c or null bytes
				 have been found.  */
	bne	cr7,L(done_null)

	mtcrf   0x01,r8

	/* Are we now aligned to a quadword boundary?  If so, skip to
	   the main loop.  Otherwise, go through the alignment code.  */

	bt	28,L(loop_null)

	/* Handle WORD2 of pair.  */
	ldu	r12,8(r8)
	cmpb    r5,r12,r0
	cmpdi	cr7,r5,0
	bne	cr7,L(done_null)
	b	L(loop_null)  /* We branch here (rather than falling through)
				 to skip the nops due to heavy alignment
				 of the loop below.  */

	/* Main loop to look for the end of the string.  Since it's a
	   small loop (< 8 instructions), align it to 32-bytes.  */
	.p2align  5
L(loop_null):
	/* Load two doublewords, compare and merge in a
	   single register for speed.  This is an attempt
	   to speed up the null-checking process for bigger strings.  */
	ld	r12,8(r8)
	ldu     r11,16(r8)
	cmpb	r5,r12,r0
	cmpb	r10,r11,r0
	or	r6,r5,r10
	cmpdi	cr7,r6,0
	beq	cr7,L(loop_null)

	/* OK, one (or both) of the doublewords contains a null byte.  Check
	   the first doubleword and decrement the address in case the first
	   doubleword really contains a null byte.  */

	cmpdi	cr6,r5,0
	addi	r8,r8,-8
	bne	cr6,L(done_null)

	/* The null byte must be in the second doubleword.  Adjust the address
	   again and move the result of cmpb to r10 so we can calculate the
	   pointer.  */

	mr	r5,r10
	addi	r8,r8,8

	/* r5 has the output of the cmpb instruction, that is, it contains
	   0xff in the same position as the null byte in the original
	   doubleword from the string.  Use that to calculate the pointer.  */
L(done_null):
	cntlzd	r0,r5	      /* Count leading zeros before the match.  */
	srdi	r0,r0,3	      /* Convert leading zeros to bytes.  */
	add	r3,r8,r0      /* Return address of the matching null byte.  */
	blr
END (BP_SYM (strchr))
weak_alias (BP_SYM (strchr), BP_SYM (index))
libc_hidden_builtin_def (strchr)
