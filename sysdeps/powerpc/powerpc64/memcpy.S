/* Optimized memcpy implementation for PowerPC64.
   Copyright (C) 2003 Free Software Foundation, Inc.
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
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include <bp-sym.h>
#include <bp-asm.h>

/* __ptr_t [r3] memcpy (__ptr_t dst [r3], __ptr_t src [r4], size_t len [r5]);
   Returns 'dst'.

   Memcpy handles short copies (< 32-bytes) using an unaligned
   word lwz/stw loop.  The tail (remaining 1-3) bytes is handled with the
   appropriate combination of byte and halfword load/stores. There is no
   attempt to optimize the alignment of short moves.  The 64-bit
   implementations of POWER3 and POWER4 do a reasonable job of handling
   unligned load/stores that do not cross 32-byte boundries.

   Longer moves (>= 32-bytes) justify the effort to get at least the
   destination doubleword (8-byte) aligned.  Further optimization is
   posible when both source and destination are doubleword aligned.
   Each case has a optimized unrolled loop.   */

EALIGN (BP_SYM (memcpy), 5, 0)
    cmpldi cr1,5,31
    neg   0,3
    std   30,-16(1)
    std   31,-8(1)
    rldicl. 0,0,0,61
    mr    12,4
    mr    31,5
    mr    30,3
    ble-  cr1,.L2
    subf  31,0,5

  /* Move 0-7 bytes as needed to get the destination doubleword alligned.  */
    beq   0f
    mtcrf 0x01,0
1:  bf    31,2f
    lbz   6,0(12)
    addi  12,12,1
    stb   6,0(3)
    addi  3,3,1
2:  bf    30,4f
    lhz   6,0(12)
    addi  12,12,2
    sth   6,0(3)
    addi  3,3,2
4:  bf    29,0f
    lwz   6,0(12)
    addi  12,12,4
    stw   6,0(3)
    addi  3,3,4
0:
  /* Copy doublewords from source to destination, assumpting the
     destination is aligned on a doubleword boundary.

     First verify that there is > 7 bytes to copy and check if the source
     is also doubleword aligned.  If there are < 8 bytes to copy fall
     through to the tail byte copy code.  Otherwise if the source and
     destination are both doubleword aligned use an optimized doubleword
     copy loop.  Otherwise the source has a different alignment and we use
     a load, shift, store strategy.  */
    rldicl. 0,12,0,61
    cmpldi cr6,31,7
    ble-  cr6,.L2  /* less than 8 bytes left.  */
    srdi  11,31,3
    andi. 10,12,7
    bne-  0,.L6   /* Source is not DW aligned.  */
    srdi. 9,31,3
    mr    10,3
    mr    11,12

  /* Move doublewords where destination and source are aligned.
     Use a unrolled loop to copy 4 doubleword (32-bytes) per iteration.
     If the remainder is >0 and < 32 bytes copy 1-3 doublewords.  */
    cmpldi	cr1,9,4
    beq   0f
    mtcrf 0x01,9
    blt   cr1,2f
    ld    6,0(11)
    .align  4
4:
    ld    7,8(11)
    addi  9,9,-4
    std   6,0(10)
    ld    6,16(11)
    std   7,8(10)
    ld    7,24(11)
    addi  11,11,32
    cmpldi	cr1,9,4
    std   6,16(10)
    blt   cr1,3f
    ld    6,0(11)
    std   7,24(10)
    addi  10,10,32
    b     4b
3:  std   7,24(10)
    addi  10,10,32
2:  bf    30,1f
    ld    6,0(11)
    ld    7,8(11)
    addi  11,11,16
    std   6,0(10)
    std   7,8(10)
    addi  10,10,16
1:  bf    31,0f
    ld    6,0(11)
    addi  11,11,8
    std   6,0(10)
    addi  10,10,8
0:

.L8:
    rldicr 0,31,0,60
    rldicl 31,31,0,61
    add   3,3,0
    add   12,12,0

	/* Copy the tail for up to 31 bytes.  If this is the tail of a longer
	   copy then the destination will be aligned and the length will be
	   less than 8.  So it is normally not worth the set-up overhead to
	   get doubleword aligned and do doubleword load/store.  */
.L2:
    mr.   10,31
    cmpldi	cr1,31,4
    beq   0f
    mtcrf 0x01,31
    blt   cr1,2f
4:  lwz   6,0(12)
    addi  12,12,4
    addi  10,10,-4
    stw   6,0(3)
    cmpldi	cr1,10,4
    addi  3,3,4
    bge   cr1,4b
2:  bf    30,1f
    lhz   6,0(12)
    addi  12,12,2
    sth   6,0(3)
    addi  3,3,2
1:  bf    31,0f
    lbz   6,0(12)
    addi  12,12,1
    stb   6,0(3)
    addi  3,3,1
0:
  /* Return original dst pointer.  */
    ld 31,-8(1)
    mr 3,30
    ld 30,-16(1)
    blr

    .align  4
.L6:

  /* Copy doublewords where the destination is aligned but the source is
     not.  Use aligned doubleword loads from the source, shifted to realign
     the data, to allow aligned destination stores.  */
    subf  5,10,12
    andi. 0,11,1
    sldi  10,10,3
    mr    4,3
    ld    6,0(5)
    ld    7,8(5)
    subfic  9,10,64
    beq   2f
    sld   0,6,10
    addi  11,11,-1
    mr    6,7
    addi  4,4,-8
    cmpldi  11,0
    b     1f
2:  addi  5,5,8
    .align  4
0:  sld   0,6,10
    srd   8,7,9
    addi  11,11,-2
    ld    6,8(5)
    or    0,0,8
    cmpldi  11,0
    std   0,0(4)
    sld   0,7,10
1:  srd   8,6,9
    or    0,0,8
    beq   8f
    ld    7,16(5)
    std   0,8(4)
    addi  5,5,16
    addi  4,4,16
    b     0b
8:
    std   0,8(4)
    b .L8
END_GEN_TB (BP_SYM (memcpy),TB_TOCLESS)
libc_hidden_builtin_def (memcpy)
