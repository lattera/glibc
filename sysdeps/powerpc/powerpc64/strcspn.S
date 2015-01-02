/* Optimized strcspn implementation for PowerPC64.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

/* size_t [r3] strcspn (const char [r4] *s, const char [r5] *reject)  */

EALIGN (strcspn, 4, 0)
	CALL_MCOUNT 3

	/* The idea to speed up the algorithm is to create a lookup table
	   for fast check if input character should be considered.  For ASCII
	   or ISO-8859-X character sets it has 256 positions.  */

	/* PPC64 ELF ABI stack is aligned to 16 bytes.  */
	addi 	r9,r1,-256
	/* Clear the table with 0 values  */
	li	r6, 0
	li	r8, 4
	mtctr	r8
	mr	r10, r9
	.align 	4
L(zerohash):
	std	r6, 0(r10)
	std	r6, 8(r10)
	std	r6, 16(r10)
	std	r6, 24(r10)
	std	r6, 32(r10)
	std	r6, 40(r10)
	std	r6, 48(r10)
	std	r6, 56(r10)
	addi	r10, r10, 64
	bdnz	L(zerohash)

	lbz	r10,0(r4)
	cmpdi	cr7,r10,0	/* reject[0] == '\0' ?  */
	li	r8,1
	beq     cr7,L(finish_table)  /* If reject[0] == '\0' skip  */

	/* Initialize the table as:
	   for (i=0; reject[i]; i++
	     table[reject[i]]] = 1  */
	.align	4
L(init_table):
	stbx	r8,r9,r10
	lbzu	r10,1(r4)
	cmpdi	cr7,r10,0           /* If reject[0] == '\0' finish  */
	bne	cr7,L(init_table)
L(finish_table):
	/* set table[0] = 1  */
	li 	r10,1
	stb	r10,0(r9)
	li	r10,0
	b	L(mainloop)

	/* Unrool the loop 4 times and check using the table as:
	   i = 0;
	   while (1)
	     {
	       if (table[input[i++]] == 1)
	         return i - 1;
	       if (table[input[i++]] == 1)
	         return i - 1;
	       if (table[input[i++]] == 1)
	         return i - 1;
	       if (table[input[i++]] == 1)
	         return i - 1;
	     }  */
	.align 4
L(unroll):
	lbz	r8,1(r3)
	addi	r10,r10,4
	lbzx	r8,r9,r8
	cmpwi	r7,r8,1
	beq	cr7,L(end)
	lbz	r8,2(r3)
	addi	r3,r3,4
	lbzx	r8,r9,r8
	cmpwi	cr7,r8,1
	beq	cr7,L(end2)
	lbz	r8,3(r7)
	lbzx	r8,r9,r8
	cmpwi	cr7,r8,1
	beq	cr7,L(end3)
L(mainloop):
	lbz	r8,0(r3)
	mr	r7,r3
	addi	r6,r10,1
	addi	r4,r10,2
	addi	r5,r10,3
	lbzx	r8,r9,8
	cmpwi	cr7,r8,1
	bne	cr7,L(unroll)
	mr	r3,r10
	blr

	.align 4
L(end):
	mr	r3,r6
	blr

	.align 4
L(end2):
	mr	r3,r4
	blr

	.align 4
L(end3):
	mr	r3,r5
	blr
END (strcspn)
libc_hidden_builtin_def (strcspn)
