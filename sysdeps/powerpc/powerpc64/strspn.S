/* Optimized strspn implementation for PowerPC64.

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

/* size_t [r3] strspn (const char *string [r3],
                       const char *needleAccept [r4]  */

/* Performance gains are grabbed through following techniques:

   > hashing of needle.
   > hashing avoids scanning of duplicate entries in needle
     across the string.
   > unrolling when scanning for character in string
     across hash table.  */

/* Algorithm is as below:
   1. A empty hash table/dictionary is created comprising of
      256 ascii character set
   2. When hash entry is found in needle , the hash index
      is initialized to 1
   3. The string is scanned until end and for every character,
      its corresponding hash index is compared.
   4. initial length of string (count) until first hit of
      accept needle to be found is set to 0
   4. If hash index is set to 1 for the index of string,
      count is returned.
   5. Otherwise count is incremented and scanning continues
      until end of string.  */

#include <sysdep.h>

EALIGN(strspn, 4, 0)
	CALL_MCOUNT 3

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
	li r8, 1		/* r8=1, marker into hash if found in
				   needle  */
	cmpdi cr7, r10, 0	/* accept needle is NULL  */
	beq cr7, L(skipHashing)	/* if needle is NULL, skip hashing  */

	.align 4		/* align section to 16 byte boundary  */
L(hashing):
	stbx r8, r9, r10	/* update hash with marker for the pivot of
				   the needle  */
	lbzu r10, 1(r4)		/* load needle into r10 and update to next  */
	cmpdi cr7, r10, 0	/* if needle is has reached NULL, continue  */
	bne cr7, L(hashing)	/* loop to hash the needle  */

L(skipHashing):
	li r10, 0		/* load counter = 0  */
	b L(beginScan)

	.align 4		/* align section to 16 byte boundary  */
L(scanUnroll):
	lbzx r8, r9, r8		/* load r8 with hash value at index  */
	cmpwi cr7, r8, 0	/* if we hit marker in hash, we have found
				   accept needle  */
	beq cr7, L(ret1stIndex)	/* we have hit accept needle, return the
				   count  */

	lbz r8, 1(r3)		/* load string[1] into r8  */
	addi r10, r10, 4	/* increment counter  */
	lbzx r8, r9, r8		/* load r8 with hash value at index  */
	cmpwi cr7, r8, 0	/* if we hit marker in hash, we have found
				   accept needle  */
	beq cr7, L(ret2ndIndex)	/* we have hit accept needle, return the
				   count  */

	lbz r8, 2(r3)		/* load string[2] into r8  */
	lbzx r8, r9, r8		/* load r8 with hash value at index  */
	cmpwi cr7, r8, 0	/* if we hit marker in hash, we have found
				   accept needle  */
	beq cr7, L(ret3rdIndex)	/* we have hit accept needle, return the
				   count  */

	lbz r8, 3(r3)		/* load string[3] into r8  */
	lbzx r8, r9, r8		/* load r8 with hash value at index  */
	addi r3, r3, 4		/* unroll factor , increment string by 4  */
	cmpwi cr7, r8, 0	/* if we hit marker in hash, we have found
				   accept needle  */
	beq cr7,L(ret4thIndex)	/* we have hit accept needle, return the
				   count  */

L(beginScan):
	lbz r8, 0(r3)		/* load string[0] into r8  */
	addi r6, r10, 1		/* place holder for counter + 1  */
	addi r5, r10, 2		/* place holder for counter + 2  */
	addi r4, r10, 3		/* place holder for counter + 3  */
	cmpdi cr7, r8, 0	/* if we hit marker in hash, we have found
				   accept needle  */
	bne cr7, L(scanUnroll)	/* continue scanning  */

L(ret1stIndex):
	mr r3, r10		/* update r3 for return  */
	blr			/* return  */

L(ret2ndIndex):
	mr r3, r6		/* update r3 for return  */
	blr			/* return  */

L(ret3rdIndex):
	mr r3, r5		/* update r3 for return  */
	blr			/* return  */

L(ret4thIndex):
	mr r3, r4		/* update r3 for return  */
	blr			/* done  */
END(strspn)
libc_hidden_builtin_def (strspn)
