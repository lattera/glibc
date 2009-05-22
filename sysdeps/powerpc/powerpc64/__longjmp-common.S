/* longjmp for PowerPC64.
   Copyright (C) 1995, 1996,1997,1999-2006,2009 Free Software Foundation, Inc.
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
#define _ASM
#define _SETJMP_H
#ifdef __NO_VMX__
# include <novmxsetjmp.h>
#else
# include <jmpbuf-offsets.h>
#endif
#include <bp-sym.h>
#include <bp-asm.h>

#ifndef __NO_VMX__
	.section	".toc","aw"
.LC__dl_hwcap:
# ifdef SHARED
	.tc _rtld_global_ro[TC],_rtld_global_ro
# else
	.tc _dl_hwcap[TC],_dl_hwcap
# endif
	.section ".text"
#endif

	.machine	"altivec"
ENTRY (BP_SYM (__longjmp))
	CALL_MCOUNT 2
	CHECK_BOUNDS_BOTH_WIDE_LIT (r3, r8, r9, JB_SIZE)
#ifndef __NO_VMX__
	ld    r5,.LC__dl_hwcap@toc(r2)
# ifdef SHARED
	/* Load _rtld-global._dl_hwcap.  */
	ld    r5,RTLD_GLOBAL_RO_DL_HWCAP_OFFSET(r5)
# else
	ld    r5,0(r5) /* Load extern _dl_hwcap.  */
# endif
	andis.  r5,r5,(PPC_FEATURE_HAS_ALTIVEC >> 16)
	beq	L(no_vmx)
	la	r5,((JB_VRS)*8)(3)
	andi.	r6,r5,0xf
	lwz	r0,((JB_VRSAVE)*8)(3)
	mtspr	VRSAVE,r0
	beq+	L(aligned_restore_vmx)
	addi    r6,r5,16
	lvsl	v0,0,r5
	lvx	v1,0,r5
	addi    r5,r5,32
	lvx	v21,0,r6
	vperm   v20,v1,v21,v0
# define load_misaligned_vmx_lo_loaded(loadvr,lovr,shiftvr,loadgpr,addgpr) \
	addi    addgpr,addgpr,32; \
	lvx	lovr,0,loadgpr; \
	vperm   loadvr,loadvr,lovr,shiftvr;
	load_misaligned_vmx_lo_loaded(v21,v22,v0,r5,r6)
	load_misaligned_vmx_lo_loaded(v22,v23,v0,r6,r5)
	load_misaligned_vmx_lo_loaded(v23,v24,v0,r5,r6)
	load_misaligned_vmx_lo_loaded(v24,v25,v0,r6,r5)
	load_misaligned_vmx_lo_loaded(v25,v26,v0,r5,r6)
	load_misaligned_vmx_lo_loaded(v26,v27,v0,r6,r5)
	load_misaligned_vmx_lo_loaded(v27,v28,v0,r5,r6)
	load_misaligned_vmx_lo_loaded(v28,v29,v0,r6,r5)
	load_misaligned_vmx_lo_loaded(v29,v30,v0,r5,r6)
	load_misaligned_vmx_lo_loaded(v30,v31,v0,r6,r5)
	lvx	v1,0,r5
	vperm   v31,v31,v1,v0
	b       L(no_vmx)
L(aligned_restore_vmx):
	addi	r6,r5,16
	lvx	v20,0,r5
	addi	r5,r5,32
	lvx	v21,0,r6
	addi	r6,r6,32
	lvx	v22,0,r5
	addi	r5,r5,32
	lvx	v23,0,r6
	addi	r6,r6,32
	lvx	v24,0,r5
	addi	r5,r5,32
	lvx	v25,0,r6
	addi	r6,r6,32
	lvx	v26,0,r5
	addi	r5,r5,32
	lvx	v27,0,r6
	addi	r6,r6,32
	lvx	v28,0,r5
	addi	r5,r5,32
	lvx	v29,0,r6
	addi	r6,r6,32
	lvx	v30,0,r5
	lvx	v31,0,r6
L(no_vmx):
#endif
#if defined PTR_DEMANGLE || defined CHECK_SP
	ld r22,(JB_GPR1*8)(r3)
#else
	ld r1,(JB_GPR1*8)(r3)
#endif
#ifdef PTR_DEMANGLE
# ifdef CHECK_SP
	PTR_DEMANGLE3 (r22, r22, r25)
# else
	PTR_DEMANGLE3 (r1, r22, r25)
# endif
#endif
#ifdef CHECK_SP
	CHECK_SP (r22)
	mr r1,r22
#endif
	ld r2,(JB_GPR2*8)(r3)
	ld r0,(JB_LR*8)(r3)
	ld r14,((JB_GPRS+0)*8)(r3)
	lfd fp14,((JB_FPRS+0)*8)(r3)
#if defined SHARED && !defined IS_IN_rtld
	std r2,40(r1)	/* Restore the callers TOC save area.  */
#endif
	ld r15,((JB_GPRS+1)*8)(r3)
	lfd fp15,((JB_FPRS+1)*8)(r3)
	ld r16,((JB_GPRS+2)*8)(r3)
	lfd fp16,((JB_FPRS+2)*8)(r3)
	ld r17,((JB_GPRS+3)*8)(r3)
	lfd fp17,((JB_FPRS+3)*8)(r3)
	ld r18,((JB_GPRS+4)*8)(r3)
	lfd fp18,((JB_FPRS+4)*8)(r3)
	ld r19,((JB_GPRS+5)*8)(r3)
	lfd fp19,((JB_FPRS+5)*8)(r3)
	ld r20,((JB_GPRS+6)*8)(r3)
	lfd fp20,((JB_FPRS+6)*8)(r3)
#ifdef PTR_DEMANGLE
	PTR_DEMANGLE2 (r0, r25)
#endif
	mtlr r0
/* 	std r2,40(r1)	Restore the TOC save area.  */
	ld r21,((JB_GPRS+7)*8)(r3)
	lfd fp21,((JB_FPRS+7)*8)(r3)
	ld r22,((JB_GPRS+8)*8)(r3)
	lfd fp22,((JB_FPRS+8)*8)(r3)
	ld r0,(JB_CR*8)(r3)
	ld r23,((JB_GPRS+9)*8)(r3)
	lfd fp23,((JB_FPRS+9)*8)(r3)
	ld r24,((JB_GPRS+10)*8)(r3)
	lfd fp24,((JB_FPRS+10)*8)(r3)
	ld r25,((JB_GPRS+11)*8)(r3)
	lfd fp25,((JB_FPRS+11)*8)(r3)
	mtcrf 0xFF,r0
	ld r26,((JB_GPRS+12)*8)(r3)
	lfd fp26,((JB_FPRS+12)*8)(r3)
	ld r27,((JB_GPRS+13)*8)(r3)
	lfd fp27,((JB_FPRS+13)*8)(r3)
	ld r28,((JB_GPRS+14)*8)(r3)
	lfd fp28,((JB_FPRS+14)*8)(r3)
	ld r29,((JB_GPRS+15)*8)(r3)
	lfd fp29,((JB_FPRS+15)*8)(r3)
	ld r30,((JB_GPRS+16)*8)(r3)
	lfd fp30,((JB_FPRS+16)*8)(r3)
	ld r31,((JB_GPRS+17)*8)(r3)
	lfd fp31,((JB_FPRS+17)*8)(r3)
	mr r3,r4
	blr
END (BP_SYM (__longjmp))
