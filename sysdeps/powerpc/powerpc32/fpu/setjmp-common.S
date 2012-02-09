/* setjmp for PowerPC.
   Copyright (C) 1995-2000, 2003-2005, 2006, 2011 Free Software Foundation, Inc.
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
#define _ASM
#ifdef __NO_VMX__
# include <novmxsetjmp.h>
#else
# include <jmpbuf-offsets.h>
#endif
#include <bp-sym.h>
#include <bp-asm.h>

	.machine	"altivec"
ENTRY (BP_SYM (__sigsetjmp))
	CHECK_BOUNDS_BOTH_WIDE_LIT (r3, r8, r9, JB_SIZE)

#ifdef PTR_MANGLE
	mr   r5,r1
	PTR_MANGLE(r5, r6)
	stw  r5,(JB_GPR1*4)(3)
#else
	stw  r1,(JB_GPR1*4)(3)
#endif
	mflr r0
	stw  r14,((JB_GPRS+0)*4)(3)
	stfd fp14,((JB_FPRS+0*2)*4)(3)
#ifdef PTR_MANGLE
	PTR_MANGLE2 (r0, r6)
#endif
	stw  r0,(JB_LR*4)(3)
	stw  r15,((JB_GPRS+1)*4)(3)
	stfd fp15,((JB_FPRS+1*2)*4)(3)
	mfcr r0
	stw  r16,((JB_GPRS+2)*4)(3)
	stfd fp16,((JB_FPRS+2*2)*4)(3)
	stw  r0,(JB_CR*4)(3)
	stw  r17,((JB_GPRS+3)*4)(3)
	stfd fp17,((JB_FPRS+3*2)*4)(3)
	stw  r18,((JB_GPRS+4)*4)(3)
	stfd fp18,((JB_FPRS+4*2)*4)(3)
	stw  r19,((JB_GPRS+5)*4)(3)
	stfd fp19,((JB_FPRS+5*2)*4)(3)
	stw  r20,((JB_GPRS+6)*4)(3)
	stfd fp20,((JB_FPRS+6*2)*4)(3)
	stw  r21,((JB_GPRS+7)*4)(3)
	stfd fp21,((JB_FPRS+7*2)*4)(3)
	stw  r22,((JB_GPRS+8)*4)(3)
	stfd fp22,((JB_FPRS+8*2)*4)(3)
	stw  r23,((JB_GPRS+9)*4)(3)
	stfd fp23,((JB_FPRS+9*2)*4)(3)
	stw  r24,((JB_GPRS+10)*4)(3)
	stfd fp24,((JB_FPRS+10*2)*4)(3)
	stw  r25,((JB_GPRS+11)*4)(3)
	stfd fp25,((JB_FPRS+11*2)*4)(3)
	stw  r26,((JB_GPRS+12)*4)(3)
	stfd fp26,((JB_FPRS+12*2)*4)(3)
	stw  r27,((JB_GPRS+13)*4)(3)
	stfd fp27,((JB_FPRS+13*2)*4)(3)
	stw  r28,((JB_GPRS+14)*4)(3)
	stfd fp28,((JB_FPRS+14*2)*4)(3)
	stw  r29,((JB_GPRS+15)*4)(3)
	stfd fp29,((JB_FPRS+15*2)*4)(3)
	stw  r30,((JB_GPRS+16)*4)(3)
	stfd fp30,((JB_FPRS+16*2)*4)(3)
	stw  r31,((JB_GPRS+17)*4)(3)
	stfd fp31,((JB_FPRS+17*2)*4)(3)
#ifndef __NO_VMX__
# ifdef PIC
	mflr    r6
	cfi_register(lr,r6)
	SETUP_GOT_ACCESS(r5,got_label)
	addis	r5,r5,_GLOBAL_OFFSET_TABLE_-got_label@ha
	addi	r5,r5,_GLOBAL_OFFSET_TABLE_-got_label@l
	mtlr	r6
	cfi_same_value (lr)
#  ifdef SHARED
	lwz     r5,_rtld_global_ro@got(r5)
	lwz     r5,RTLD_GLOBAL_RO_DL_HWCAP_OFFSET+4(r5)
#  else
	lwz     r5,_dl_hwcap@got(r5)
	lwz     r5,4(r5)
#  endif
# else
	lis	r6,(_dl_hwcap+4)@ha
	lwz     r5,(_dl_hwcap+4)@l(r6)
# endif
	andis.	r5,r5,(PPC_FEATURE_HAS_ALTIVEC >> 16)
	beq	L(no_vmx)
	la	r5,((JB_VRS)*4)(3)
	andi.	r6,r5,0xf
	mfspr	r0,VRSAVE
	stw	r0,((JB_VRSAVE)*4)(3)
	addi	r6,r5,16
	beq+	L(aligned_save_vmx)
	lvsr	v0,0,r5
	vspltisb v1,-1         /* set v1 to all 1's */
	vspltisb v2,0          /* set v2 to all 0's */
	vperm   v3,v2,v1,v0   /* v3 contains shift mask with num all 1 bytes on left = misalignment  */


	/* Special case for v20 we need to preserve what is in save area below v20 before obliterating it */
	lvx     v5,0,r5
	vperm   v20,v20,v20,v0
	vsel    v5,v5,v20,v3
	vsel    v20,v20,v2,v3
	stvx    v5,0,r5

#define save_2vmx_partial(savevr,prev_savevr,hivr,shiftvr,maskvr,savegpr,addgpr) \
	addi    addgpr,addgpr,32; \
	vperm   savevr,savevr,savevr,shiftvr; \
	vsel    hivr,prev_savevr,savevr,maskvr; \
	stvx    hivr,0,savegpr;

	save_2vmx_partial(v21,v20,v5,v0,v3,r6,r5)
	save_2vmx_partial(v22,v21,v5,v0,v3,r5,r6)
	save_2vmx_partial(v23,v22,v5,v0,v3,r6,r5)
	save_2vmx_partial(v24,v23,v5,v0,v3,r5,r6)
	save_2vmx_partial(v25,v24,v5,v0,v3,r6,r5)
	save_2vmx_partial(v26,v25,v5,v0,v3,r5,r6)
	save_2vmx_partial(v27,v26,v5,v0,v3,r6,r5)
	save_2vmx_partial(v28,v27,v5,v0,v3,r5,r6)
	save_2vmx_partial(v29,v28,v5,v0,v3,r6,r5)
	save_2vmx_partial(v30,v29,v5,v0,v3,r5,r6)

	/* Special case for r31 we need to preserve what is in save area above v31 before obliterating it */
	addi    r5,r5,32
	vperm   v31,v31,v31,v0
	lvx     v4,0,r5
	vsel    v5,v30,v31,v3
	stvx    v5,0,r6
	vsel    v4,v31,v4,v3
	stvx    v4,0,r5
	b	L(no_vmx)

L(aligned_save_vmx):
	stvx	20,0,r5
	addi	r5,r5,32
	stvx	21,0,r6
	addi	r6,r6,32
	stvx	22,0,r5
	addi	r5,r5,32
	stvx	23,0,r6
	addi	r6,r6,32
	stvx	24,0,r5
	addi	r5,r5,32
	stvx	25,0,r6
	addi	r6,r6,32
	stvx	26,0,r5
	addi	r5,r5,32
	stvx	27,0,r6
	addi	r6,r6,32
	stvx	28,0,r5
	addi	r5,r5,32
	stvx	29,0,r6
	addi	r6,r6,32
	stvx	30,0,r5
	stvx	31,0,r6
L(no_vmx):
#endif
	b BP_SYM (__sigjmp_save@local)
END (BP_SYM (__sigsetjmp))
