/* Optimized strcmp implementation for Power7 using 'cmpb' instruction
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

/* The optimization is achieved here through cmpb instruction.
   8byte aligned strings are processed with double word comparision
   and unaligned strings are handled effectively with loop unrolling
   technique  */

#include <sysdep.h>

/* int [r3] strcmp (const char *s1 [r3], const char *s2 [r4])  */

EALIGN (strcmp, 4, 0)
	CALL_MCOUNT 2

	or r9, r3, r4
	rldicl. r10, r9, 0, 61	/* are s1 and s2 8 byte aligned..?  */
	bne cr0, L(process_unaligned_bytes)

/* process input parameters on double word aligned boundary  */
	ld r9, 0(r4)		/* load s2 at offset=0  */
	li r10, 0		/* load mask=0  */
	cmpb r10, r9, r10	/* compare bytes at s2 with mask  */
	cmpdi cr7, r10, 0	/* is NULL found ..? is end of string HIT  */
	bne cr7, L(process_unaligned_bytes)	/* process byte by byte  */

	ld r10, 0(r3)		/* load s1 at offset=0  */
	li r8, 0		/* load mask=0  */
	cmpb r8, r10, r8	/* compare bytes at s1 with mask  */
	cmpdi cr7, r8, 0	/* is NULL found ..? is end of string HIT  */
	bne cr7, L(process_unaligned_bytes)	/* process byte by byte  */

/*s1 and s2 does not contain NULL now , so compare all 8 bytes in a GO  */
	cmpb r9, r10, r9	/* compare s1 and s2  */
	cmpdi cr7, r9, -1	/* compare result with 0xFFFFFFFFFFFFFFFF  */
	bne cr7, L(process_unaligned_bytes)	/* s1,s2 mismatch found  */

	addi r5, r3, 8		/* save next offset of s2  */
	addi r11, r4, 8		/* save next offset of s1  */
	ld r8, 8(r4)		/* load s2 at offset=8  */
	li r9, 0		/* load mask=0  */
	cmpb r9, r8, r9		/* compare bytes at s2 with mask  */
	cmpdi cr7, r9, 0	/* NULL found ..?  */
	bne cr7, L(processBytes)/* update input and process bytes one by one  */

	mr r9, r4		/* save s2  */
	li r10, 0		/* load mask=0  */

	ld r7, 8(r3)		/* load s1 at offset=8  */
	cmpb r6, r7, r10	/* compare bytes at s1 with mask  */
	cmpdi cr7, r6, 0	/* is NULL found  */
	bne cr7, L(processBytes)/* mismatch, so process one by one  */

L(unrollDword):
	cmpb r8, r7, r8		/* compare s1 and s2  */
	cmpdi cr7, r8, -1	/* compare result with 0xFFFFFFFFFFFFFFFF  */
	bne cr7, L(processBytes)/* mismatch with s1 and s2  */

	addi r5, r3, 16		/* save offset=16 of s1  */
	addi r4, r9, 16		/* save offset=16 of s2  */
	ld r8, 16(r9)		/* load s2 at offset=16  */
	cmpb r7, r8, r10	/* compare bytes at s2 with mask  */
	cmpdi cr7, r7, 0	/* NULL found  ..?  */
	bne cr7, L(update2processBytes)

	ld r7, 16(r3)		/* load s1 at offset=16  */
	cmpb r6, r7, r10	/* check s1 for end of string  */
	cmpdi cr7, r6, 0	/* end of s1 ?,then handle byte by byte  */
	bne 7,L(update2processBytes)

	cmpb r8, r7, r8		/* compare s1 and s2 double words  */
	cmpdi cr7, r8, -1	/* compare results with 0xFFFFFFFFFFFFFFFF  */
	bne cr7,L(update2processBytes)

	addi r5, r3, 24		/* update s1 to offset=24  */
	addi r4, r9, 24		/* update s2 to offset=24  */

	ld r8, 24(r9)		/* load s2  */
	cmpb r7, r8, r10	/* compare s2 for NULL  */
	cmpdi cr7, r7, 0	/* verify if s2 is ending now  */
	bne cr7,L(update2processBytes)

	ld r7, 24(r3)		/* load s1 at offset=24  */
	cmpb r6, r7, r10	/* verify for NULL  */
	cmpdi cr7, r6, 0	/* is NULL found  */
	bne cr7, L(update2processBytes)

	cmpb r8, r7, r8		/* compare s1 and s2  */
	cmpdi cr7, r8, -1	/* are s1 and s2 same ..?  */
	bne cr7, L(update2processBytes)

	addi r7, r9, 32		/* update s2 to next double word  */
	addi r3, r3, 32		/* update s1 to next double word  */

	ld r8, 32(r9)		/* load s2  */
	mr r4, r7		/* save s2  */
	cmpb r6, r8, r10	/* compare s2 with NULL  */
	cmpdi cr7, r6, 0	/* end of s2 ..? */
	bne cr7, L(process_unaligned_bytes)

	ld r6, 0(r3)		/* load and compare s1 for NULL  */
	cmpb r5, r6, r10
	cmpdi cr7, r5, 0
	bne cr7, L(process_unaligned_bytes)

	cmpb r8, r6, r8		/* compare s1 and s2  */
	cmpdi cr7, r8, -1
	bne cr7, L(process_unaligned_bytes)

	addi r5, r3, 8		/* increment s1 and d2 here  */
	addi r11, r9, 40

	ld r8, 40(r9)		/* process s2 now  */
	cmpb r9, r8, r10
	cmpdi cr7, r9, 0
	bne cr7, L(processBytes)

	mr r9, r7
	ld r7, 8(r3)		/* process s1 now  */
	cmpb r6, r7, r10
	cmpdi cr7, r6, 0
	beq cr7, L(unrollDword)	/* unroll to compare s1 and s2  */

L(processBytes):
	mr r4, r11		/* update input params  */
	mr r3, r5

	.p2align 4
L(process_unaligned_bytes):
	lbz r9, 0(r3)		/* load byte from s1  */
	lbz r10, 0(r4)		/* load byte from s2  */
	cmpdi cr7, r9, 0	/* compare *s1 with NULL  */
	beq cr7, L(diffOfNULL)	/* if *s1 is NULL , return *s1 - *s2  */
	cmplw cr7, r9, r10	/* compare *s1 and *s2  */
	bne cr7, L(ComputeDiff)	/* branch to compute difference and return  */

	lbz r9, 1(r3)		/* load next byte from s1  */
	lbz r10, 1(r4)		/* load next byte from s2  */
	cmpdi cr7, r9, 0	/* compare *s1 with NULL  */
	beq cr7, L(diffOfNULL)	/* if *s1 is NULL , return *s1 - *s2  */
	cmplw cr7, r9, r10	/* compare *s1 and *s2  */
	bne cr7, L(ComputeDiff)	/* branch to compute difference and return  */

	lbz r9, 2(r3)		/* unroll 3rd byte here  */
	lbz r10, 2(r4)
	cmpdi cr7, r9, 0
	beq cr7, L(diffOfNULL)
	cmplw cr7, r9, r10
	bne 7, L(ComputeDiff)

	lbz r9, 3(r3)		/* unroll 4th byte now  */
	lbz r10, 3(r4)
	addi r3, r3, 4		/* increment s1 by unroll factor  */
	cmpdi cr7, r9, 0
	cmplw cr6, 9, r10
	beq cr7, L(diffOfNULL)
	addi r4, r4, 4		/* increment s2 by unroll factor  */
	beq cr6, L(process_unaligned_bytes)	/* unroll byte processing  */

	.p2align 4
L(ComputeDiff):
	extsw r9, r9
	subf r10, r10, r9	/* compute s1 - s2  */
	extsw r3, r10
	blr			/* return  */

	.p2align 4
L(diffOfNULL):
	li r9, 0
	subf r10, r10, r9	/* compute s1 - s2  */
	extsw r3, r10		/* sign extend result  */
	blr			/* return  */

	.p2align 4
L(update2processBytes):
	mr r3, r5		/* update and proceed  */
	b L(process_unaligned_bytes)

END (strcmp)
libc_hidden_builtin_def (strcmp)
