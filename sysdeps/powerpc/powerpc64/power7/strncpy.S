/* Copyright (C) 2014-2015 Free Software Foundation, Inc.
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

/* Implements the functions

   char * [r3] strncpy (char *dst [r3], const char *src [r4], size_t n [r5])

   AND

   char * [r3] stpncpy (char *dst [r3], const char *src [r4], size_t n [r5])

   The algorithm is as follows:
   > if src and dest are 8 byte aligned, perform double word copy
     else
   > copy byte by byte on unaligned addresses.

   The aligned comparison are made using cmpb instructions.  */

/* The focus on optimization for performance improvements are as follows:
   1. data alignment [gain from aligned memory access on read/write]
   2. POWER7 gains performance with loop unrolling/unwinding
      [gain by reduction of branch penalty].
   3. The final pad with null bytes is done by calling an optimized
      memset.  */

#ifdef USE_AS_STPNCPY
# define FUNC_NAME __stpncpy
#else
# define FUNC_NAME strncpy
#endif

#define		FRAMESIZE	(FRAME_MIN_SIZE+32)

#ifndef MEMSET
/* For builds with no IFUNC support, local calls should be made to internal
   GLIBC symbol (created by libc_hidden_builtin_def).  */
# ifdef SHARED
#  define MEMSET   __GI_memset
# else
#  define MEMSET   memset
# endif
#endif

	.machine  power7
EALIGN(FUNC_NAME, 4, 0)
	CALL_MCOUNT 3

	mflr r0			/* load link register LR to r0  */
	or r10, r3, r4		/* to verify source and destination  */
	rldicl. r8, r10, 0, 61	/* is double word aligned .. ?  */

	std r19, -8(r1)		/* save callers register , r19  */
	std r18, -16(r1)	/* save callers register , r18  */
	std r0, 16(r1)		/* store the link register  */
	stdu r1, -FRAMESIZE(r1)	/* create the stack frame  */

	mr r9, r3		/* save r3 into r9 for use  */
	mr r18, r3		/* save r3 for retCode of strncpy  */
	bne 0, L(byte_by_byte)


	srdi r11, r5, 3		/* compute count for CTR ; count = n/8  */
	cmpldi cr7, r11, 3	/* if count > 4 ; perform unrolling 4 times  */
	ble 7, L(update1)

	ld r10, 0(r4)		/* load doubleWord from src  */
	cmpb r8, r10, r8	/* compare src with NULL ,we read just now  */
	cmpdi cr7, r8, 0	/* if cmpb returned NULL ; we continue  */
	bne cr7, L(update3)

	std r10, 0(r3)		/* copy doubleword at offset=0  */
	ld r10, 8(r4)		/* load next doubleword from offset=8  */
	cmpb r8, r10, r8	/* compare src with NULL , we read just now  */
	cmpdi cr7, r8, 0	/* if cmpb returned NULL ; we continue  */
	bne 7,L(HopBy8)

	addi r8, r11, -4
	mr r7, r3
	srdi r8, r8, 2
	mr r6, r4
	addi r8, r8, 1
	li r12, 0
	mtctr r8
	b L(dwordCopy)

	.p2align 4
L(dWordUnroll):
	std r8, 16(r9)
	ld r8, 24(r4)		/* load dword,perform loop unrolling again  */
	cmpb r10, r8, r10
	cmpdi cr7, r10, 0
	bne cr7, L(HopBy24)

	std r8, 24(r7)		/* copy dword at offset=24  */
	addi r9, r9, 32
	addi r4, r4, 32
	bdz  L(leftDwords)	/* continue with loop on counter  */

	ld r3, 32(r6)
	cmpb r8, r3, r10
	cmpdi cr7, r8, 0
	bne cr7, L(update2)

	std r3, 32(r7)
	ld r10, 40(r6)
	cmpb r8, r10, r8
	cmpdi cr7, r8, 0
	bne cr7, L(HopBy40)

	mr r6, r4		/* update values  */
	mr r7, r9
	mr r11, r0
	mr r5, r19

L(dwordCopy):
	std r10, 8(r9)		/* copy dword at offset=8  */
	addi r19, r5, -32
	addi r0, r11, -4
	ld r8, 16(r4)
	cmpb r10, r8, r12
	cmpdi cr7, r10, 0
	beq cr7, L(dWordUnroll)

	addi r9, r9, 16		/* increment dst by 16  */
	addi r4, r4, 16		/* increment src by 16  */
	addi r5, r5, -16	/* decrement length 'n' by 16  */
	addi r0, r11, -2	/* decrement loop counter  */

L(dWordUnrollOFF):
	ld r10, 0(r4)		/* load first dword  */
	li r8, 0		/* load mask  */
	cmpb r8, r10, r8
	cmpdi cr7, r8, 0
	bne cr7, L(byte_by_byte)
	mtctr r0
	li r7, 0
	b L(CopyDword)

	.p2align 4
L(loadDWordandCompare):
	ld r10, 0(r4)
	cmpb r8, r10, r7
	cmpdi cr7, r8, 0
	bne cr7, L(byte_by_byte)

L(CopyDword):
	addi r9, r9, 8
	std r10, -8(r9)
	addi r4, r4, 8
	addi r5, r5, -8
	bdnz L(loadDWordandCompare)

L(byte_by_byte):
	cmpldi cr7, r5, 3
	ble cr7, L(verifyByte)
	srdi r10, r5, 2
	mr r19, r9
	mtctr r10
	b L(firstByteUnroll)

	.p2align 4
L(bytes_unroll):
	lbz r10, 1(r4)		/* load byte from src  */
	cmpdi cr7, r10, 0	/* compare for NULL  */
	stb r10, 1(r19)		/* store byte to dst  */
	beq cr7, L(updtDestComputeN2ndByte)

	addi r4, r4, 4		/* advance src  */

	lbz r10, -2(r4)		/* perform loop unrolling for byte r/w  */
	cmpdi cr7, r10, 0
	stb r10, 2(r19)
	beq cr7, L(updtDestComputeN3rdByte)

	lbz r10, -1(r4)		/* perform loop unrolling for byte r/w  */
	addi r19, r19, 4
	cmpdi cr7, r10, 0
	stb r10, -1(r19)
	beq cr7, L(ComputeNByte)

	bdz L(update0)

L(firstByteUnroll):
	lbz r10, 0(r4)		/* perform loop unrolling for byte r/w  */
	cmpdi cr7, 10, 0
	stb r10, 0(r19)
	bne cr7, L(bytes_unroll)
	addi r19, r19, 1

L(ComputeNByte):
	subf r9, r19, r9	/* compute 'n'n bytes to fill  */
	add r8, r9, r5

L(zeroFill):
	cmpdi cr7, r8, 0	/* compare if length is zero  */
	beq cr7, L(update3return)

	mr r3, r19		/* fill buffer with  */
	li r4, 0		/* zero fill buffer  */
	mr r5, r8		/* how many bytes to fill buffer with  */
	bl MEMSET		/* call optimized memset  */
	nop

L(update3return):
#ifdef USE_AS_STPNCPY
	addi r3, r19, -1	/* update return value  */
#endif

L(hop2return):
#ifndef USE_AS_STPNCPY
	mr r3, r18		/* set return value  */
#endif
	addi r1, r1, FRAMESIZE	/* restore stack pointer  */
	ld r0, 16(r1)		/* read the saved link register  */
	ld r18, -16(r1)		/* restore callers save register, r18  */
	ld r19, -8(r1)		/* restore callers save register, r19  */
	mtlr r0			/* branch to link register  */
	blr			/* return  */

	.p2align 4
L(update0):
	mr r9, r19

	.p2align 4
L(verifyByte):
	rldicl. r8, r5, 0, 62
#ifdef USE_AS_STPNCPY
	mr r3, r9
#endif
	beq cr0, L(hop2return)
	mtctr r8
	addi r4, r4, -1
	mr r19, r9
	b L(oneBYone)

	.p2align 4
L(proceed):
	bdz L(done)

L(oneBYone):
	lbzu r10, 1(r4)		/* copy byte  */
	addi r19, r19, 1
	addi r8, r8, -1
	cmpdi cr7, r10, 0
	stb r10, -1(r19)
	bne cr7, L(proceed)
	b L(zeroFill)

	.p2align 4
L(done):
	addi r1, r1, FRAMESIZE	/* restore stack pointer  */
#ifdef USE_AS_STPNCPY
	mr r3, r19		/* set the return value  */
#else
	mr r3, r18		/* set the return value  */
#endif
	ld r0, 16(r1)		/* read the saved link register  */
	ld r18, -16(r1)		/* restore callers save register, r18  */
	ld r19, -8(r1)		/* restore callers save register, r19  */
	mtlr r0			/* branch to link register  */
	blr			/* return  */

L(update1):
	mr r0, r11
	mr r19, r5

	.p2align 4
L(leftDwords):
	cmpdi cr7, r0, 0
	mr r5, r19
	bne cr7, L(dWordUnrollOFF)
	b L(byte_by_byte)

	.p2align 4
L(updtDestComputeN2ndByte):
	addi r19, r19, 2	/* update dst by 2  */
	subf r9, r19, r9	/* compute distance covered  */
	add r8, r9, r5
	b L(zeroFill)

	.p2align 4
L(updtDestComputeN3rdByte):
	addi r19, r19, 3	/* update dst by 3  */
	subf r9, r19, r9	/* compute distance covered  */
	add r8, r9, r5
	b L(zeroFill)

	.p2align 4
L(HopBy24):
	addi r9, r9, 24		/* increment dst by 24  */
	addi r4, r4, 24		/* increment src by 24  */
	addi r5, r5, -24	/* decrement length 'n' by 24  */
	addi r0, r11, -3	/* decrement loop counter  */
	b L(dWordUnrollOFF)

	.p2align 4
L(update2):
	mr r5, r19
	b L(dWordUnrollOFF)

	.p2align 4
L(HopBy40):
	addi r9, r7, 40		/* increment dst by 40  */
	addi r4, r6, 40		/* increment src by 40  */
	addi r5, r5, -40	/* decrement length 'n' by 40  */
	addi r0, r11, -5	/* decrement loop counter  */
	b L(dWordUnrollOFF)

L(update3):
	mr r0, r11
	b L(dWordUnrollOFF)

L(HopBy8):
	addi r9, r3, 8		/* increment dst by 8  */
	addi r4, r4, 8		/* increment src by 8  */
	addi r5, r5, -8		/* decrement length 'n' by 8  */
	addi r0, r11, -1	/* decrement loop counter  */
	b L(dWordUnrollOFF)
END(FUNC_NAME)
#ifndef USE_AS_STPNCPY
libc_hidden_builtin_def (strncpy)
#endif
