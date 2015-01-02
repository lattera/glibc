/* Optimized strtok implementation for PowerPC64.

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
      accept needle is calculated and moved.(strspn)
   5. The string is again scanned until end and for every character,
      its corresponding hash index is compared.(strpbrk)
   6. If hash index is set to 1 for the index of string,
      set it to null and set the saveptr to point to the next char.
   7. Otherwise count is incremented and scanning continues
      until end of string.  */

#include <sysdep.h>
#ifdef USE_AS_STRTOK_R
# define FUNC_NAME __strtok_r
#else
# define FUNC_NAME strtok
#endif

EALIGN(FUNC_NAME, 4, 0)
#ifdef USE_AS_STRTOK_R
	CALL_MCOUNT	3
	cmpdi	cr7, r3, 0		/* Is input null? */
	bne	cr7, L(inputnotNull)
	ld	r3, 0(r5)		/* Load from r5 */
#else
	CALL_MCOUNT	2
	addis	r5, r2, .LANCHOR0@toc@ha
	cmpdi	cr7, r3, 0		/* Is r3 NULL? */
	bne	cr7, L(inputnotNull)
	ld	r3, .LANCHOR0@toc@l(r5)	/* Load from saveptr */
#endif
L(inputnotNull):
	mr	r7, r3
	cmpdi	cr7, r3, 0
	beq	cr7, L(returnNULL)
	lbz	r8, 0(r3)
	cmpdi	cr7, r8, 0
	beq	cr7, L(returnNULL)

	addi	r9, r1, -256	/* r9 is a hash of 256 bytes  */

	/*Iniatliaze hash table with Zeroes */
	li	r6, 0
	li	r8, 4
	mtctr	r8
	mr	r10, r9
	.align	4
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


	lbz	r10, 0(r4)	/* load r10 with needle (r4)  */
	li	r8, 1		/* r8=1, marker into hash if found in
				   needle  */

	cmpdi	cr7, r10, 0	/* accept needle is NULL  */
	beq	cr7, L(skipHashing)	/* if needle is NULL, skip hashing  */

	.align 4		/* align section to 16 byte boundary  */
L(hashing):
	stbx	r8, r9, r10	/* update hash with marker for the pivot of
				   the needle  */
	lbzu	r10, 1(r4)	/* load needle into r10 and update to next  */
	cmpdi	cr7, r10, 0	/* if needle is has reached NULL, continue  */
	bne	cr7, L(hashing)	/* loop to hash the needle  */

L(skipHashing):
	b	L(beginScan)

	.align 4		/* align section to 16 byte boundary  */
L(scanUnroll):
	lbzx	r8, r9, r8	/* load r8 with hash value at index  */
	cmpwi	cr7, r8, 0	/* check the hash  value */
	beq	cr7, L(ret1stIndex)	/* we have hit accept needle */

	lbz	r8, 1(r7)	/* load string[1] into r8  */
	lbzx	r8, r9, r8	/* load r8 with hash value at index  */
	cmpwi	cr7, r8, 0	/* check the hash  value */
	beq	cr7, L(ret2ndIndex)	/* we have hit accept needle */

	lbz	r8, 2(r7)	/* load string[1] into r8  */
	lbzx	r8, r9, r8	/* load r8 with hash value at index  */
	cmpwi	cr7, r8, 0	/* check the hash  value */
	beq	cr7, L(ret3rdIndex)	/* we have hit accept needle */

	lbz	r8, 3(r7)	/* load string[1] into r8  */
	addi	r7, r7, 4
	lbzx	r8, r9, r8	/* load r8 with hash value at index  */
	cmpwi	cr7, r8, 0	/* check the hash  value */
	beq	cr7,L(ret4thIndex)	/* we have hit accept needle */

L(beginScan):
	lbz	r8, 0(r7)	/* load string[0] into r8  */
	addi	r6, r7, 1
	addi	r11, r7, 2
	addi	r4, r7, 3
	cmpdi	cr7, r8, 0	/*  check if its null */
	bne	cr7, L(scanUnroll)	/* continue scanning  */

L(ret1stIndex):
	mr 	r3, r7
	b 	L(next)
L(ret2ndIndex):
	mr 	r3, r6
	b 	L(next)
L(ret3rdIndex):
	mr 	r3, r11
	b 	L(next)
L(ret4thIndex):
	mr 	r3, r4
L(next):
	mr	r7, r3
	lbz	r8, 0(r7)
	cmpdi	cr7, r8, 0
	beq	cr7, L(returnNULL)
	li	r8, 1
	li	r10, 0		/* load counter = 0  */
	stbx	r8, r9, r10	/* update hash for NULL */
	b	L(mainloop)

L(unroll):
	lbz	r8, 1(r7)	/* load string[1] into r8  */
	lbzx	r8, r9, r8	/* load r8 with hash value at index  */
	cmpwi	r7, r8, 1	/* check the hash */
	beq	cr7, L(foundat1st)	/* we have hit accept needle */
	lbz	r8, 2(r7)
	lbzx	r8, r9, r8
	cmpwi	cr7, r8, 1
	beq	cr7, L(foundat2nd)
	lbz	r8, 3(r7)
	addi	r7, r7, 4
	lbzx	r8, r9, r8
	cmpwi	cr7, r8, 1
	beq	cr7, L(foundat3rd)
L(mainloop):
	lbz	r8, 0(r7)
	addi	r6, r7, 1
	addi	r11, r7, 2
	addi	r4, r7, 3
	lbzx	r8, r9, r8
	cmpwi	cr7, r8, 1
	bne	cr7, L(unroll)	/* continue scanning  */

	b	L(found)
L(foundat1st):
	mr	r7, r6
	b	L(found)
L(foundat2nd):
	mr	r7, r11
	b	L(found)
L(foundat3rd):
	mr	r7, r4
L(found):
	lbz	r8, 0(r7)
	cmpdi	cr7, r8, 0
	beq	cr7, L(end)
	li	r10, 0
	stb	r10, 0(r7)	/* Terminate string */
	addi	r7, r7, 1	/* Store the pointer to the next char */
L(end):
#ifdef USE_AS_STRTOK_R
	std	r7, 0(r5)	/* Update saveptr */
#else
	std	r7, .LANCHOR0@toc@l(r5)
#endif
	blr			/* done  */
L(returnNULL):
#ifndef USE_AS_STRTOK_R
	li	r7, 0
#endif
	li	r3, 0		/* return NULL */
	b	L(end)
END(FUNC_NAME)
#ifdef USE_AS_STRTOK_R
libc_hidden_builtin_def (strtok_r)
#else
	.section        ".bss"
	.align 3
	.set    .LANCHOR0,. + 0
	.type   olds, @object
	.size   olds, 8
olds:
	.zero   8
libc_hidden_builtin_def (strtok)
#endif
