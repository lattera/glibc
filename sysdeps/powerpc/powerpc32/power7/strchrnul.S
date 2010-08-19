/* Optimized strchrnul implementation for PowerPC32/POWER7 using cmpb insn.
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

/* int [r3] strchrnul (char *s [r3], int c [r4])  */
	.machine  power7
ENTRY (BP_SYM(__strchrnul))
	CALL_MCOUNT
	dcbt	0,r3
	clrrwi	r8,r3,2	      /* Align the address to word boundary.  */

	/* Replicate byte to word.  */
	rlwimi	r4,r4,8,16,23
	rlwimi	r4,r4,16,0,15

	rlwinm	r6,r3,3,27,28 /* Calculate padding.  */
	lwz	r12,0(r8)     /* Load word from memory.  */
	li	r0,0	      /* Word with null chars to use
				 with cmpb.  */

	/* Now r4 has a word of c bytes and r0 has
	   a word of null bytes.  */

	cmpb	r10,r12,r0    /* Compare each byte against c byte.  */
	cmpb	r9,r12,r4     /* Compare each byte against null byte.  */

	/* Move the words left and right to discard the bits that are
	   not part of the string and bring them back as zeros.  */
	slw	r10,r10,r6
	slw	r9,r9,r6
	srw	r10,r10,r6
	srw	r9,r9,r6
	or	r5,r9,r10     /* OR the results to speed things up.  */
	cmpwi	cr7,r5,0      /* If r5 == 0, no c or null bytes
				 have been found.  */
	bne	cr7,L(done)

	mtcrf   0x01,r8

	/* Are we now aligned to a quadword boundary?  If so, skip to
	   the main loop.  Otherwise, go through the alignment code.  */

	bt	29,L(loop)

	/* Handle WORD2 of pair.  */
	lwzu	r12,4(r8)
	cmpb	r10,r12,r0
	cmpb	r9,r12,r4
	or	r5,r9,r10
	cmpwi	cr7,r5,0
	bne	cr7,L(done)
	b	L(loop)	      /* We branch here (rather than falling through)
				 to skip the nops due to heavy alignment
				 of the loop below.  */

	.p2align  5
L(loop):
	/* Load two words, compare and merge in a
	   single register for speed.  This is an attempt
	   to speed up the null-checking process for bigger strings.  */
	lwz	r12,4(r8)
	lwzu     r11,8(r8)
	cmpb	r10,r12,r0
	cmpb	r9,r12,r4
	cmpb	r6,r11,r0
	cmpb	r7,r11,r4
	or	r5,r9,r10
	or	r10,r6,r7
	or	r11,r5,r10
	cmpwi	cr7,r11,0
	beq	cr7,L(loop)

	/* OK, one (or both) of the words contains a c/null byte.  Check
	   the first word and decrement the address in case the first
	   word really contains a c/null byte.  */

	cmpwi	cr6,r5,0
	addi	r8,r8,-4
	bne	cr6,L(done)

	/* The c/null byte must be in the second word.  Adjust the
	   address again and move the result of cmpb to r10 so we can calculate
	   the pointer.  */
	mr	r5,r10
	addi	r8,r8,4

	/* r5 has the output of the cmpb instruction, that is, it contains
	   0xff in the same position as the c/null byte in the original
	   word from the string.  Use that to calculate the pointer.  */
L(done):
	cntlzw	r0,r5	      /* Count leading zeros before the match.  */
	srwi	r0,r0,3	      /* Convert leading zeros to bytes.  */
	add	r3,r8,r0      /* Return address of matching c/null byte.  */
	blr
END (BP_SYM (__strchrnul))
weak_alias (__strchrnul,strchrnul)
libc_hidden_builtin_def (__strchrnul)
