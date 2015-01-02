/* Optimized strpbrk implementation for PowerPC64.
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

/* char [r3] *strpbrk(const char [r4] *s, const char [r5] *accept)  */

EALIGN (strpbrk, 4, 0)
	CALL_MCOUNT 3

	lbz	r10,0(r4)
	cmpdi	cr7,r10,0	/* accept[0] == '\0' ?  */
	beq	cr7,L(nullfound)

	/* The idea to speed up the algorithm is to create a lookup table
	   for fast check if input character should be considered.  For ASCII
	   or ISO-8859-X character sets it has 256 positions.  */

	/* PPC64 ELF ABI stack is aligned to 16 bytes.  */
	addi 	r9,r1,-256
	/* Clear the table with 0 values  */
	li	r6, 0
	li	r7, 4
	mtctr	r7
	mr	r8, r9
	.align 	4
L(zerohash):
	std	r6, 0(r8)
	std	r6, 8(r8)
	std	r6, 16(r8)
	std	r6, 24(r8)
	std	r6, 32(r8)
	std	r6, 40(r8)
	std	r6, 48(r8)
	std	r6, 56(r8)
	addi	r8, r8, 64
	bdnz	L(zerohash)

	/* Initialize the table as:
	   for (i=0; accept[i]; i++
	     table[accept[i]]] = 1  */
	li      r0,1
	.align 4
L(init_table):
	stbx	r0,r9,r10
	lbzu	r10,1(r4)
	cmpdi	r0,r10,0
	bne	cr0,L(init_table)
L(finish_table):
	/* set table[0] = 1  */
	li	r4,1
	stb	r4,0(r9)
	b	L(mainloop)

	/* Unrool the loop 4 times and check using the table as:
	   i = 0;
	   while (1)
	     {
	       if (table[input[i++]] == 1)
	         return (s[i -1] ? s + i - 1: NULL);
	       if (table[input[i++]] == 1)
	         return (s[i -1] ? s + i - 1: NULL);
	       if (table[input[i++]] == 1)
	         return (s[i -1] ? s + i - 1: NULL);
	       if (table[input[i++]] == 1)
	         return (s[i -1] ? s + i - 1: NULL);
	     }  */
	.align 4
L(unroll):
	lbz	r0,1(r3)
	lbzx	r8,r9,r0
	cmpwi	cr6,r8,1
	beq	cr6,L(checkend2)
	lbz	r10,2(r3)
	lbzx	r4,r9,r10
	cmpwi	cr7,r4,1
	beq	cr7,L(checkend3)
	lbz	r12,3(r3)
	addi	r3,r3,4
	lbzx	r11,r9,r12
	cmpwi	cr0,r11,1
	beq	cr0,L(checkend)
L(mainloop):
	lbz	r12,0(r3)
	addi	r11,r3,1
	addi	r5,r3,2
	addi	r7,r3,3
	lbzx	r6,r9,r12
	cmpwi	cr1,r6,1
	bne	cr1,L(unroll)
	cmpdi	cr0,r12,0
	beq	cr0,L(nullfound)
L(end):
	blr

	.align 4
L(checkend):
	cmpdi	cr1,r12,0
	mr	r3,r7
	bne	cr1,L(end)
L(nullfound):
	/* return NULL  */
	li 3,0
	blr

	.align 4
L(checkend2):
	cmpdi	cr7,r0,0
	mr	r3,r11
	beq	cr7,L(nullfound)
	blr

	.align 4
L(checkend3):
	cmpdi	cr6,r10,0
	mr	r3,r5
	beq	cr6,L(nullfound)
	blr
END (strpbrk)
libc_hidden_builtin_def (strpbrk)
