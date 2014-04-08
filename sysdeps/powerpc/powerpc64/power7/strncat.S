/* Optimized strncat implementation for PowerPC64/POWER7.

   Copyright (C) 2014 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* The algorithm is as follows for aligned memory access :

   if address of s2 is divisible by 0x7UL,
       perform aligned doubleword catenation
   else
       perform unaligned catenation

   The aligned comparison are made using cmpb instructions.  */

/* char* [r3] strncat (const char *s1 [r3],
                       const char *s2 [r4],
                       size_t size [r5])  */

#include <sysdep.h>

#ifndef STRNCAT
# undef strncat
# define STRNCAT  strncat
#endif

#ifndef STRLEN
/* For builds with no IFUNC support, local calls should be made to internal
   GLIBC symbol (created by libc_hidden_builtin_def).  */
# define STRLEN   __GI_strlen
#endif

#define	FRAMESIZE	(FRAME_MIN_SIZE+32)

	.machine  power7
EALIGN(STRNCAT, 4, 0)
	CALL_MCOUNT 3

	mflr r0				/* Load link register LR to r0.  */

/* We shall use r29, r30 and r31 non volatile register for retention.
   Save all the callee registers in the GPR save area.  */
	std r29, -24(r1)		/* Save callers register r29.  */
	std r30, -16(r1)		/* Save callers register r30.  */
	std r31, -8(r1)			/* Save callers register r31.  */

	std r0, 16(r1)			/* Store the link register.  */
	stdu r1, -FRAMESIZE(r1)		/* Create the stack frame.  */

/* Improve performance with CPU pre-fetch.  */
	dcbt 0, r3			/* Pre-fetch str to avoid cache
					   miss.  */
	dcbt 0, r4			/* Pre-fetch accept to avoid cache
					   miss.  */

	mr. r29, r5			/* Save "n" in r29.  */
	mr r30, r3			/* Save "s1" in r30 from r3.  */
	beq cr0,L(done)

	mr r31, r4			/* Save "s2" in r31 from r4.  */
	bl STRLEN			/* Call optimized strlen on s1; goto
					   end of s1.  */
	nop
	cmpldi cr7, r29, 7		/* If s2 is <=7 process
					    byte-by-byte.  */
	add r3, r30, r3			/* Grab the last character of s1.  */
	bgt cr7,L(alignment)		/* Process by aligned strings.  */

	cmpldi cr7, r29, 3		/* If n is >= 4, we can
					   byte-unroll.  */
	addi r9, r3, -1			/* Make "s1" point before next
					   character, increment when read.  */
	bgt cr7, L(bytes_unroll)	/* Process each byte.  */

L(byte_by_byte):
	lbz r10, 0(r31)
	addi r8, r9, 1
	cmpdi cr7, r10, 0		/* Check for NULL in "s2".  */
	stb r10, 1(r9)
	beq cr7, L(done)
	add r9, r9, r29
	subf r9, r8, r9
	addi r9, r9, 1
	mtctr r9
	b L(branch2)
	.p2align 4
L(branch1):
	lbzu r10, 1(r31)
	cmpdi cr7, r10, 0
	stbu r10, 1(r8)
	beq cr7,L(done)
L(branch2):
	mr r9, r8
	bdnz L(branch1)
	beq cr7,L(done)
L(nullTerminate):
	li r10, 0			/* Load NULL for termination.  */
	stb r10, 1(r9)			/* Append or terminate s1 with
					   NULL.  */
	.p2align 4			/* A small section here.  */
L(done):				/* We return now.   */
	addi r1, r1, FRAMESIZE		/* Restore stack pointer.  */
	mr r3, r30			/* Set the return value length of
					   string.  */
	ld r0, 16(r1)			/* Read the saved link register.  */
	ld r29, -24(r1)			/* Restore save register r29.  */
	ld r30, -16(r1)			/* Restore save register r30.  */
	ld r31, -8(r1)			/* Restore save register r31.  */
	mtlr r0				/* Restore link register.  */
	blr				/* Branch to link register.  */

	.p2align 4
L(alignment):
	rldicl. r9, r31, 0, 61		/* Check if s2 is 8byte aligned  */
	beq cr0,L(dwordAligned)

	.p2align 4
/* Unaligned bytes in string, so process byte by byte.
   POWER7 has performance gains over loop unroll.  */
L(bytes_unroll):
	addi r9, r3, -1
	srdi r10, r29, 2
	mtctr r10
	b L(L10)
	.p2align 4
L(L44):
	lbz r10, 1(r31)			/* Load byte.  */
	cmpdi cr7, r10, 0		/* Compare ; if byte not zero,
					   continue.  */
	stb r10, 2(r9)			/* Store byte  */
	beq cr7, L(done)
	addi r31, r31, 4

	lbz r10, -2(r31)		/* Perform loop unroll here on byte
					   load and store.  */
	cmpdi cr7, r10, 0
	stb r10, 3(r9)
	beq cr7, L(done)

	lbz r10, -1(r31)		/* Loop unroll here.  */
	cmpdi cr7, r10, 0
	stbu r10, 4(r9)
	beq cr7, L(done)

	bdz L(leftNbytes)

L(L10):
	lbz r10, 0(r31)			/* Loop unroll here.  */
	cmpdi cr7, r10, 0
	stb r10, 1(r9)
	bne cr7,L(L44)
	b L(done)
	.p2align 4
/* If s2 is double word aligned, we load and store double word.  */
L(dwordAligned):
/* read, write 8 bytes at a time  */
	srdi r8, r29, 3			/* Compute count for CTR to loop;
					   count = n/8.  */
	li r7, 0			/* Load r7 with NULL.  */
	li r10, 0			/* Load r10 with MASK '0'.  */

	mtctr r8			/* Move count to CTR.  */
L(loop8):
	ld r9, 0(r31)			/* Read double word from s2.  */
	cmpb r6, r9, r10		/* Compare bytes in s2 we read
					   just now.  */
	cmpdi r6, 0			/* If cmpb returned NULL,
					   we continue.  */
	bne+ L(a8)
	std r9, 0(r3)			/* Append double word from s2
					   with s1.  */
	addi r3, r3, 8			/* Increment s1.  */
	addi r31, r31, 8		/* Increment s2.  */
	subi r29, r29, 8		/* Decrement count by 8.  */
	bdnz L(loop8)			/* Continue until "count" is
					   non zero.  */

L(a8):
	cmpdi r29, 0			/* If "n" is already zero, we skip. */
	beq+ L(align8align)

	mtctr r29			/* Process left over bytes in "n".  */
L(unaligned0):
	lbz r9, 0(r31)			/* Read a byte from s2.  */
	cmpw r9, r7			/* If byte is NULL, we stop here . */
	beq+ L(align8align)		/* Skip processing further if NULL.  */
	stb  r9, 0(r3)			/* If not NULL, store byte into s1.  */
	addi r3, r3, 1			/* Increment s1 by 1.  */
	addi r31, r31, 1		/* Increment s2 by 1.  */
	bdnz L(unaligned0)		/* Decrement counter "n" and loop
					   until non zero.  */
L(align8align):
	stb r7, 0(r3)			/* Terminate s1 with NULL.  */

	addi r1, r1, FRAMESIZE		/* Restore stack pointer.  */
	mr r3, r30			/* Set the return value, length of
					   string.  */
	ld r0, 16(r1)			/* Read the saved link register.  */
	ld r29, -24(r1)			/* Restore save register r29.  */
	ld r30, -16(r1)			/* Restore save register r30.  */
	ld r31, -8(r1)			/* Restore save register r31.  */
	mtlr r0				/* Restore link register.  */
	blr				/* Branch to link register  */

	.p2align 4
L(leftNbytes):
	rldicl. r29, r29, 0, 62		/* Check if n>0 and n < 4 bytes.  */
	bne cr0,L(byte_by_byte)		/* Process bytes one by one. */
	b L(nullTerminate)		/* Now, finish catenation with
					   NULL termination.  */
END(STRNCAT)
