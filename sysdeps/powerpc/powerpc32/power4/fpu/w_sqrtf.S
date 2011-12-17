/* sqrtf function.  PowerPC32 version.
   Copyright (C) 2007, 2011 Free Software Foundation, Inc.
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
   Software Foundation, Inc., 1 Franklin Street, Fifth Floor, Boston MA
   02110-1301 USA.  */

#include <sysdep.h>
#include <math_ldbl_opt.h>

/* float [fp1] sqrts (float x [fp1])
   Power4 (ISA V2.0) and above implement sqrt in hardware (not optional).
   The fsqrts instruction generates the correct value for all inputs and
   sets the appropriate floating point exceptions.  Extented checking is
   only needed to set errno (via __kernel_standard) if the input value
   is negative.
   
   The fsqrts will set FPCC and FU (Floating Point Unordered or NaN
   to indicated that the input value was negative or NaN. Use Move to
   Condition Register from FPSCR to copy the FPCC field to cr1.  The
   branch on summary overflow transfers control to w_sqrt to process
   any error conditions. Otherwise we can return the result directly.
   
   This part of the function is a leaf routine,  so no need to stack a
   frame or execute prologue/epilogue code. This means it is safe to
   transfer directly to w_sqrt as long as the input value (f1) is
   preserved. Putting the sqrt result into f2 (float parameter 2)
   allows passing both the input value and sqrt result into the extended
   wrapper so there is no need to recompute.
   
   This tactic avoids the overhead of stacking a frame for the normal
   (non-error) case.  Until gcc supports prologue shrink-wrapping
   this is the best we can do.  */

	.section	".text"
	.machine power4
EALIGN (__sqrtf, 5, 0)
	fsqrts	fp2,fp1
	mcrfs	cr1,4
	bso-	cr1,.Lw_sqrtf
	fmr	fp1,fp2
	blr
        .align 4
.Lw_sqrtf:
	mflr	r0
	stwu	r1,-16(r1)
	cfi_adjust_cfa_offset(16)
	fmr	fp12,fp2
	stw	r0,20(r1)
	stw	r30,8(r1)
	cfi_offset(lr,20-16)
	cfi_offset(r30,8-16)
#ifdef SHARED
	SETUP_GOT_ACCESS(r30,got_label)
	addis	r30,r30,_GLOBAL_OFFSET_TABLE_-got_label@ha
	addi	r30,r30,_GLOBAL_OFFSET_TABLE_-got_label@l
	lwz	r9,_LIB_VERSION@got(30)
	lwz	r0,0(r9)
#else
	lis	r9,_LIB_VERSION@ha
	lwz	r0,_LIB_VERSION@l(r9)
#endif
/*  if (_LIB_VERSION == _IEEE_) return z; */
	cmpwi	cr7,r0,-1
	beq-	cr7,.L4
/*  if (x != x, 0) return z; !isnan */
	fcmpu	cr7,fp1,fp1
	bne-	cr7,.L4
/*  if  (x < 0.0)
    return __kernel_standard (x, x, 126) */
	fmr	fp2,fp1
	fabs	fp0,fp1
	li	r3,126
	fcmpu	cr7,1,0
	bne- 	cr7,.L11
.L4:
	lwz	r0,20(r1)
	fmr	fp1,fp12
	lwz	r30,8(r1)
	addi	r1,r1,16
	mtlr 	r0
	blr
.L11:
	bl	__kernel_standard@plt
	fmr	fp12,fp1
	b	.L4
	END	(__sqrtf)

weak_alias (__sqrtf, sqrtf)

