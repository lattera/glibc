/* PLT trampolines.  PPC64 version.
   Copyright (C) 2005 Free Software Foundation, Inc.
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

	.section ".text"

EALIGN(_dl_runtime_resolve, 4, 0)
/* We need to save the registers used to pass parameters, ie. r3 thru
   r10; the registers are saved in a stack frame.  */
	stdu	r1,-128(r1)
	std	r3,48(r1)
	mr	r3,r11
	std	r4,56(r1)
	sldi	r4,r0,1
	std	r5,64(r1)
	add	r4,r4,r0
	std	r6,72(r1)
	sldi	r4,r4,3
	std	r7,80(r1)
	mflr	r0
	std	r8,88(r1)
/* Store the LR in the LR Save area of the previous frame.  */
	std	r0,128+16(r1)
	mfcr	r0
	std	r9,96(r1)
	std	r10,104(r1)
/* I'm almost certain we don't have to save cr...  be safe.  */
	std	r0,8(r1)
	bl	JUMPTARGET(_dl_fixup)
/* Put the registers back.  */
	ld	r0,128+16(r1)
	ld	r10,104(r1)
	ld	r9,96(r1)
	ld	r8,88(r1)
	ld	r7,80(r1)
	mtlr	r0
	ld	r0,8(r1)
	ld	r6,72(r1)
	ld	r5,64(r1)
	ld	r4,56(r1)
	mtcrf	0xFF,r0
/* Load the target address, toc and static chain reg from the function
   descriptor returned by fixup.  */
	ld	r0,0(r3)
	ld	r2,8(r3)
	mtctr	r0
	ld	r11,16(r3)
	ld	r3,48(r1)
/* Unwind the stack frame, and jump.  */
	addi	r1,r1,128
	bctr
END(_dl_runtime_resolve)



EALIGN(_dl_profile_resolve, 4, 0)
/* We need to save the registers used to pass parameters, ie. r3 thru
   r10; the registers are saved in a stack frame.  */
	stdu	1,-128(1)
	std	3,48(1)
	mr	3,11
	std	4,56(1)
	sldi	4,0,1
	std	5,64(1)
	add	4,4,0
	std	6,72(1)
	sldi	4,4,3
	std	7,80(1)
	mflr	0
	std	8,88(1)
/* Store the LR in the LR Save area of the previous frame.  */
	std	0,128+16(1)
	mfcr	0
	std	9,96(1)
	std	10,104(1)
/* I'm almost certain we don't have to save cr...  be safe.  */
	std	0,8(1)
	bl	JUMPTARGET(_dl_profile_fixup)
/* Put the registers back.  */
	ld	0,128+16(1)
	ld	10,104(1)
	ld	9,96(1)
	ld	8,88(1)
	ld	7,80(1)
	mtlr	0
	ld	0,8(1)
	ld	6,72(1)
	ld	5,64(1)
	ld	4,56(1)
	mtcrf	0xFF,0
/* Load the target address, toc and static chain reg from the function
   descriptor returned by fixup.  */
	ld	0,0(3)
	ld	2,8(3)
	mtctr	0
	ld	11,16(3)
	ld	3,48(1)
/* Unwind the stack frame, and jump.  */
	addi	1,1,128
	bctr
END(_dl_profile_resolve)
