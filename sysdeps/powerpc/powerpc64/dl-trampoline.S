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
	stdu	r1,-448(r1)
	/* Stack layout:

	  +432   stackframe
	  +424   lr
	  +416   r1
	  +400   v12
	  +384   v11
	  +368   v10
	  +362   v9
	  +336   v8
	  +320   v7
	  +304   v6
	  +288   v5
	  +272   v4
	  +256   v3
	  +240   v2
	  +224   v1
	  +216   free
	  +208   fp13
	  +200   fp12
	  +192   fp11
	  +184   fp10
	  +176   fp9
	  +168   fp8
	  +160   fp7
	  +152   fp6
	  +144   fp5
	  +136   fp4
	  +128   fp3
	  +120   fp2
	  +112   fp1
	  +104   r10
	  +96    r9
	  +88    r8
	  +80    r7
	  +72    r6
	  +64    r5
	  +56    r4
	  +48    r3
	  +8     cr
	   r1    link
	*/
	std	r3,48(r1)
	mr	r3,r11
	std	r4,56(r1)
	sldi	r4,r0,1
	std	r5,64(r1)
	add	r4,r4,0
	std	r6,72(r1)
	sldi	r4,r4,3
	std	r7,80(r1)
	mflr	r5
	std	r8,88(r1)
/* Store the LR in the LR Save area of the previous frame.  */
/* XXX Do we have to do this?  */
	std	r5,448+16(r1)
	std	r5,424(r1)
	mfcr	r0
	std	r9,96(r1)
	std	r10,104(r1)
/* I'm almost certain we don't have to save cr...  be safe.  */
	std	r0,8(r1)
/* Save floating registers.  */
	stfd	fp1,112(r1)
	stfd	fp2,120(r1)
	stfd	fp3,128(r1)
	stfd	fp4,136(r1)
	stfd	fp5,144(r1)
	stfd	fp6,152(r1)
	stfd	fp7,160(r1)
	stfd	fp8,168(r1)
	stfd	fp9,176(r1)
	stfd	fp10,184(r1)
	stfd	fp11,192(r1)
	stfd	fp12,200(r1)
	stfd	fp13,208(r1)
/* XXX TODO: store vmx registers.  */
/* Load the extra parameters.  */
	addi	r6,r1,48
	addi	r7,r1,432
	li	r0,-1
	stdu	r0,0(r7)
	bl	JUMPTARGET(_dl_profile_fixup)
/* Put the registers back.  */
	ld	r0,448+16(r1)
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
/* Load the floating point registers.  */
	lfd	fp1,112(r1)
	lfd	fp2,120(r1)
	lfd	fp3,128(r1)
	lfd	fp4,136(r1)
	lfd	fp5,144(r1)
	lfd	fp6,152(r1)
	lfd	fp7,160(r1)
	lfd	fp8,168(r1)
	lfd	fp9,176(r1)
	lfd	fp10,184(r1)
	lfd	fp11,192(r1)
	lfd	fp12,200(r1)
	lfd	fp13,208(r1)
/* Unwind the stack frame, and jump.  */
	addi	r1,r1,448
	bctr
END(_dl_profile_resolve)
