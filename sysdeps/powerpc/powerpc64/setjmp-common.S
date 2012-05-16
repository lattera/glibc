/* setjmp for PowerPC64.
   Copyright (C) 1995-2012 Free Software Foundation, Inc.
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
#include <novmxsetjmp.h>
#else
#include <jmpbuf-offsets.h>
#endif
#include <bp-sym.h>
#include <bp-asm.h>

#ifndef __NO_VMX__
	.section	".toc","aw"
.LC__dl_hwcap:
# ifdef SHARED
#  ifdef IS_IN_rtld
	/* Inside ld.so we use the local alias to avoid runtime GOT
	   relocations.  */
	.tc _rtld_local_ro[TC],_rtld_local_ro
#  else
	.tc _rtld_global_ro[TC],_rtld_global_ro
#  endif
# else
	.tc _dl_hwcap[TC],_dl_hwcap
# endif
	.section ".text"
#endif

	.machine	"altivec"
ENTRY (setjmp)
	CALL_MCOUNT 1
	li r4,1			/* Set second argument to 1.  */
	b JUMPTARGET (GLUE(__sigsetjmp,_ent))
END (setjmp)

#if defined SHARED && !defined IS_IN_rtld && !defined __NO_VMX__
/* When called from within libc we need a special version of _setjmp
   that saves r2 since the call won't go via a plt call stub.  See
   bugz #269.  __GI__setjmp is used in csu/libc-start.c when
   HAVE_CLEANUP_JMP_BUF is defined.  */
ENTRY (BP_SYM (__GI__setjmp))
	std r2,40(r1)		/* Save the callers TOC in the save area.  */
	cfi_endproc
END_2 (BP_SYM (__GI__setjmp))
/* Fall thru. */
#endif

ENTRY (BP_SYM (_setjmp))
	CALL_MCOUNT 1
	li r4,0			/* Set second argument to 0.  */
	b JUMPTARGET (GLUE(__sigsetjmp,_ent))
END (BP_SYM (_setjmp))
libc_hidden_def (_setjmp)

ENTRY (BP_SYM (__sigsetjmp))
	CALL_MCOUNT 2
JUMPTARGET(GLUE(__sigsetjmp,_ent)):
	CHECK_BOUNDS_BOTH_WIDE_LIT (r3, r8, r9, JB_SIZE)
#ifdef PTR_MANGLE
	mr   r5, r1
	PTR_MANGLE (r5, r6)
	std  r5,(JB_GPR1*8)(3)
#else
	std  r1,(JB_GPR1*8)(3)
#endif
	mflr r0
#if defined SHARED && !defined IS_IN_rtld
	ld   r5,40(r1)	/* Retrieve the callers TOC.  */
	std  r5,(JB_GPR2*8)(3)
#else
	std  r2,(JB_GPR2*8)(3)
#endif
	std  r14,((JB_GPRS+0)*8)(3)
	stfd fp14,((JB_FPRS+0)*8)(3)
#ifdef PTR_MANGLE
	PTR_MANGLE2 (r0, r6)
#endif
	std  r0,(JB_LR*8)(3)
	std  r15,((JB_GPRS+1)*8)(3)
	stfd fp15,((JB_FPRS+1)*8)(3)
	mfcr r0
	std  r16,((JB_GPRS+2)*8)(3)
	stfd fp16,((JB_FPRS+2)*8)(3)
	std  r0,(JB_CR*8)(3)
	std  r17,((JB_GPRS+3)*8)(3)
	stfd fp17,((JB_FPRS+3)*8)(3)
	std  r18,((JB_GPRS+4)*8)(3)
	stfd fp18,((JB_FPRS+4)*8)(3)
	std  r19,((JB_GPRS+5)*8)(3)
	stfd fp19,((JB_FPRS+5)*8)(3)
	std  r20,((JB_GPRS+6)*8)(3)
	stfd fp20,((JB_FPRS+6)*8)(3)
	std  r21,((JB_GPRS+7)*8)(3)
	stfd fp21,((JB_FPRS+7)*8)(3)
	std  r22,((JB_GPRS+8)*8)(3)
	stfd fp22,((JB_FPRS+8)*8)(3)
	std  r23,((JB_GPRS+9)*8)(3)
	stfd fp23,((JB_FPRS+9)*8)(3)
	std  r24,((JB_GPRS+10)*8)(3)
	stfd fp24,((JB_FPRS+10)*8)(3)
	std  r25,((JB_GPRS+11)*8)(3)
	stfd fp25,((JB_FPRS+11)*8)(3)
	std  r26,((JB_GPRS+12)*8)(3)
	stfd fp26,((JB_FPRS+12)*8)(3)
	std  r27,((JB_GPRS+13)*8)(3)
	stfd fp27,((JB_FPRS+13)*8)(3)
	std  r28,((JB_GPRS+14)*8)(3)
	stfd fp28,((JB_FPRS+14)*8)(3)
	std  r29,((JB_GPRS+15)*8)(3)
	stfd fp29,((JB_FPRS+15)*8)(3)
	std  r30,((JB_GPRS+16)*8)(3)
	stfd fp30,((JB_FPRS+16)*8)(3)
	std  r31,((JB_GPRS+17)*8)(3)
	stfd fp31,((JB_FPRS+17)*8)(3)
#ifndef __NO_VMX__
	ld    r6,.LC__dl_hwcap@toc(r2)
# ifdef SHARED
	/* Load _rtld-global._dl_hwcap.  */
	ld    r6,RTLD_GLOBAL_RO_DL_HWCAP_OFFSET(r6)
# else
	ld    r6,0(r6) /* Load extern _dl_hwcap.  */
# endif
	andis.  r6,r6,(PPC_FEATURE_HAS_ALTIVEC >> 16)
	beq	L(no_vmx)
	la	r5,((JB_VRS)*8)(3)
	andi.	r6,r5,0xf
	mfspr	r0,VRSAVE
	stw	r0,((JB_VRSAVE)*8)(3)
	addi	r6,r5,16
	beq+	L(aligned_save_vmx)
	lvsr	v0,0,r5
	vspltisb v1,-1         /* set v1 to all 1's */
	vspltisb v2,0          /* set v2 to all 0's */
	vperm   v3,v2,v1,v0   /* v3 contains shift mask with num all 1 bytes
				 on left = misalignment  */


	/* Special case for v20 we need to preserve what is in save area
	   below v20 before obliterating it */
	lvx     v5,0,r5
	vperm   v20,v20,v20,v0
	vsel    v5,v5,v20,v3
	vsel    v20,v20,v2,v3
	stvx    v5,0,r5

# define save_2vmx_partial(savevr,prev_savevr,hivr,shiftvr,maskvr,savegpr,addgpr) \
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

	/* Special case for r31 we need to preserve what is in save area
	   above v31 before obliterating it */
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
#else
	li	r6,0
#endif
#if defined NOT_IN_libc && defined IS_IN_rtld
	li	r3,0
	blr
#elif defined SHARED
	b	JUMPTARGET (BP_SYM (__sigjmp_save))
#else
	mflr	r0
	std	r0,16(r1)
	stdu	r1,-112(r1)
	cfi_adjust_cfa_offset(112)
	cfi_offset(lr,16)
	bl	JUMPTARGET (BP_SYM (__sigjmp_save))
	nop
	ld	r0,112+16(r1)
	addi	r1,r1,112
	mtlr	r0
	blr
#endif
END (BP_SYM (__sigsetjmp))
