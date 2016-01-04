/* Optimized strcpy/stpcpy implementation for PowerPC64/POWER8.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

#ifdef USE_AS_STPCPY
# define FUNC_NAME __stpcpy
#else
# define FUNC_NAME strcpy
#endif

/* Implements the function

   char * [r3] strcpy (char *dest [r3], const char *src [r4])

   or

   char * [r3] stpcpy (char *dest [r3], const char *src [r4])

   if USE_AS_STPCPY is defined.

   The implementation uses unaligned doubleword access to avoid specialized
   code paths depending of data alignment.  Although recent powerpc64 uses
   64K as default, the page cross handling assumes minimum page size of
   4k.  */

	.machine  power7
EALIGN (FUNC_NAME, 4, 0)
        li      r0,0          /* Doubleword with null chars to use
                                 with cmpb.  */

	/* Check if the [src]+15 will cross a 4K page by checking if the bit
	   indicating the page size changes.  Basically:

	   uint64_t srcin = (uint64_t)src;
	   uint64_t ob = srcin & 4096UL;
	   uint64_t nb = (srcin+15UL) & 4096UL;
	   if (ob ^ nb)
	     goto pagecross;  */

	addi	r9,r4,15
	xor	r9,r9,r4
	rlwinm.	r9,r9,0,19,19
	bne	L(pagecross)

	/* For short string (less than 16 bytes), just calculate its size as
	   strlen and issues a memcpy if null is found.  */
	mr	r7,r4
        ld      r12,0(r7)     /* Load doubleword from memory.  */
        cmpb    r10,r12,r0    /* Check for null bytes in DWORD1.  */
        cmpdi   cr7,r10,0     /* If r10 == 0, no null's have been found.  */
        bne     cr7,L(done)

        ldu     r8,8(r7)
        cmpb    r10,r8,r0
        cmpdi   cr7,r10,0
        bne     cr7,L(done)

	b	L(loop_before)

	.align	4
L(pagecross):
	clrrdi  r7,r4,3       /* Align the address to doubleword boundary.  */
	rlwinm  r6,r4,3,26,28 /* Calculate padding.  */
	li      r5,-1         /* MASK = 0xffffffffffffffff.  */
        ld      r12,0(r7)     /* Load doubleword from memory.  */
#ifdef __LITTLE_ENDIAN__
        sld     r5,r5,r6
#else
        srd     r5,r5,r6      /* MASK = MASK >> padding.  */
#endif
        orc     r9,r12,r5     /* Mask bits that are not part of the string.  */
        cmpb    r10,r9,r0     /* Check for null bytes in DWORD1.  */
        cmpdi   cr7,r10,0     /* If r10 == 0, no null's have been found.  */
        bne     cr7,L(done)

        ldu     r6,8(r7)
        cmpb    r10,r6,r0
        cmpdi   cr7,r10,0
        bne     cr7,L(done)

        ld      r12,0(r7)
        cmpb    r10,r12,r0
        cmpdi   cr7,r10,0
        bne     cr7,L(done)

        ldu     r6,8(r7)
        cmpb    r10,r6,r0
        cmpdi   cr7,r10,0
        bne     cr7,L(done)

	/* We checked for 24 - x bytes, with x being the source alignment
	   (0 <= x <= 16), and no zero has been found.  Start the loop
	   copy with doubleword aligned address.  */
	mr	r7,r4
	ld	r12, 0(r7)
	ldu	r8, 8(r7)

L(loop_before):
	/* Save the two doublewords readed from source and align the source
	   to 16 bytes for the loop.  */
	mr	r11,r3
	std	r12,0(r11)
	std	r8,8(r11)
	addi	r11,r11,16
	rldicl	r9,r4,0,60
	subf	r7,r9,r7
	subf	r11,r9,r11
	b	L(loop_start)

        .align  5
L(loop):
        std     r12, 0(r11)
        std     r6, 8(r11)
	addi	r11,r11,16
L(loop_start):
        /* Load two doublewords, compare and merge in a
           single register for speed.  This is an attempt
           to speed up the null-checking process for bigger strings.  */

        ld      r12, 8(r7)
        ldu     r6, 16(r7)
        cmpb    r10,r12,r0
        cmpb    r9,r6,r0
        or      r8,r9,r10     /* Merge everything in one doubleword.  */
        cmpdi   cr7,r8,0
        beq     cr7,L(loop)


        /* OK, one (or both) of the doublewords contains a null byte.  Check
           the first doubleword and decrement the address in case the first
           doubleword really contains a null byte.  */

	addi	r4,r7,-8
        cmpdi   cr6,r10,0
        addi    r7,r7,-8
        bne     cr6,L(done2)

        /* The null byte must be in the second doubleword.  Adjust the address
           again and move the result of cmpb to r10 so we can calculate the
           length.  */

        mr      r10,r9
        addi    r7,r7,8
	b	L(done2)

        /* r10 has the output of the cmpb instruction, that is, it contains
           0xff in the same position as the null byte in the original
           doubleword from the string.  Use that to calculate the length.  */
L(done):
	mr	r11,r3
L(done2):
#ifdef __LITTLE_ENDIAN__
        addi    r9, r10, -1   /* Form a mask from trailing zeros.  */
        andc    r9, r9, r10
        popcntd r6, r9        /* Count the bits in the mask.  */
#else
        cntlzd  r6,r10        /* Count leading zeros before the match.  */
#endif
        subf    r5,r4,r7
        srdi    r6,r6,3       /* Convert leading/trailing zeros to bytes.  */
        add     r8,r5,r6      /* Compute final length.  */
#ifdef USE_AS_STPCPY
	/* stpcpy returns the dest address plus the size not counting the
	   final '\0'.  */
	add	r3,r11,r8
#endif
	addi	r8,r8,1       /* Final '/0'.  */

	cmpldi	cr6,r8,8
	mtocrf	0x01,r8
	ble	cr6,L(copy_LE_8)

	cmpldi	cr1,r8,16
	blt	cr1,8f

	/* Handle copies of 0~31 bytes.  */
	.align	4
L(copy_LT_32):
	/* At least 6 bytes to go.  */
	blt	cr1,8f

	/* Copy 16 bytes.  */
	ld	r6,0(r4)
	ld	r8,8(r4)
	addi	r4,r4,16
	std	r6,0(r11)
	std	r8,8(r11)
	addi	r11,r11,16
8:	/* Copy 8 bytes.  */
	bf	28,L(tail4)
	ld	r6,0(r4)
	addi	r4,r4,8
	std	r6,0(r11)
	addi	r11,r11,8

	.align	4
/* Copies 4~7 bytes.  */
L(tail4):
	bf	29,L(tail2)
	lwz	r6,0(r4)
	stw	r6,0(r11)
	bf	30,L(tail5)
	lhz	r7,4(r4)
	sth	r7,4(r11)
	bflr	31
	lbz	r8,6(r4)
	stb	r8,6(r11)
	blr

	.align	4
/* Copies 2~3 bytes.  */
L(tail2):
	bf	30,1f
	lhz	r6,0(r4)
	sth	r6,0(r11)
	bflr	31
	lbz	r7,2(r4)
	stb	r7,2(r11)
	blr

	.align	4
L(tail5):
	bf	31,1f
	lbz	r6,4(r4)
	stb	r6,4(r11)
	blr

	.align	4
1:
	bflr	31
	lbz	r6,0(r4)
	stb	r6,0(r11)
	blr

/* Handles copies of 0~8 bytes.  */
	.align	4
L(copy_LE_8):
	bne	cr6,L(tail4)
	ld	r6,0(r4)
	std	r6,0(r11)
	blr
END (FUNC_NAME)

#ifndef USE_AS_STPCPY
libc_hidden_builtin_def (strcpy)
#endif
