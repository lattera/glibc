/* Jump to a new context.
   Copyright (C) 2002, 2004 Free Software Foundation, Inc.
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
#include <rtld-global-offsets.h>
#include <shlib-compat.h>

#define __ASSEMBLY__
#include <asm/ptrace.h>
#include "ucontext_i.h"

	.machine	"altivec"
ENTRY(__setcontext)
	mflr	r0
	stwu	r1,-16(r1)
	stw	r0,20(r1)
	stw	r31,12(r1)
	lwz	r31,_UC_REGS_PTR(r3)

	/*
	 * If this ucontext refers to the point where we were interrupted
	 * by a signal, we have to use the rt_sigreturn system call to
	 * return to the context so we get both LR and CTR restored.
	 *
	 * Otherwise, the context we are restoring is either just after
	 * a procedure call (getcontext/swapcontext) or at the beginning
	 * of a procedure call (makecontext), so we don't need to restore
	 * r0, xer, ctr.  We don't restore r2 since it will be used as
	 * the TLS pointer.
	 */
	lwz	r0,_UC_GREGS+(PT_MSR*4)(r31)
	cmpwi	r0,0
	bne	L(do_sigret)

	/* Restore the signal mask */
	li	r5,0
	addi	r4,r3,_UC_SIGMASK
	li	r3,SIG_SETMASK
	bl	JUMPTARGET(__sigprocmask)
	cmpwi	r3,0
	bne	L(error_exit)

#ifdef PIC
	mflr    r8
	bl      _GLOBAL_OFFSET_TABLE_@local-4
	mflr    r7
# ifdef SHARED
	lwz     r7,_rtld_global_ro@got(r7)
	mtlr    r8
	lwz     r7,RTLD_GLOBAL_RO_DL_HWCAP_OFFSET(r7)
# else
	lwz     r7,_dl_hwcap@got(r7)
	mtlr    r8
	lwz     r7,0(r7)
# endif
#else
	lis	r7,_dl_hwcap@ha
	lwz     r7,_dl_hwcap@l(r7)
#endif
	andis.	r7,r7,(PPC_FEATURE_HAS_ALTIVEC >> 16)
	la	r10,(_UC_VREGS)(r31)
	beq	L(has_no_vec)

	lwz   r0,(32*16)(r10)
	li    r9,(32*16)
	cmpwi r0,0
	mtspr VRSAVE,r0
	beq   L(has_no_vec)

	lvx   v19,r9,r10
	la    r9,(16)(r10)

	lvx   v0,0,r10
	lvx   v1,0,r9
	addi  r10,r10,32
	addi  r9,r9,32

	mtvscr  v19
	lvx   v2,0,r10
	lvx   v3,0,r9
	addi  r10,r10,32
	addi  r9,r9,32

	lvx   v4,0,r10
	lvx   v5,0,r9
	addi  r10,r10,32
	addi  r9,r9,32

	lvx   v6,0,r10
	lvx   v7,0,r9
	addi  r10,r10,32
	addi  r9,r9,32

	lvx   v8,0,r10
	lvx   v9,0,r9
	addi  r10,r10,32
	addi  r9,r9,32

	lvx   v10,0,r10
	lvx   v11,0,r9
	addi  r10,r10,32
	addi  r9,r9,32

	lvx   v12,0,r10
	lvx   v13,0,r9
	addi  r10,r10,32
	addi  r9,r9,32

	lvx   v14,0,r10
	lvx   v15,0,r9
	addi  r10,r10,32
	addi  r9,r9,32

	lvx   v16,0,r10
	lvx   v17,0,r9
	addi  r10,r10,32
	addi  r9,r9,32

	lvx   v18,0,r10
	lvx   v19,0,r9
	addi  r10,r10,32
	addi  r9,r9,32

	lvx   v20,0,r10
	lvx   v21,0,r9
	addi  r10,r10,32
	addi  r9,r9,32

	lvx   v22,0,r10
	lvx   v23,0,r9
	addi  r10,r10,32
	addi  r9,r9,32

	lvx   v24,0,r10
	lvx   v25,0,r9
	addi  r10,r10,32
	addi  r9,r9,32

	lvx   v26,0,r10
	lvx   v27,0,r9
	addi  r10,r10,32
	addi  r9,r9,32

	lvx   v28,0,r10
	lvx   v29,0,r9
	addi  r10,r10,32
	addi  r9,r9,32

	lvx   v30,0,r10
	lvx   v31,0,r9
	addi  r10,r10,32
	addi  r9,r9,32

	lvx   v10,0,r10
	lvx   v11,0,r9

L(has_no_vec):
	/* Restore the floating-point registers */
	lfd	fp31,_UC_FREGS+(32*8)(r31)
	lfd	fp0,_UC_FREGS+(0*8)(r31)
	mtfsf	0xff,fp31
	lfd	fp1,_UC_FREGS+(1*8)(r31)
	lfd	fp2,_UC_FREGS+(2*8)(r31)
	lfd	fp3,_UC_FREGS+(3*8)(r31)
	lfd	fp4,_UC_FREGS+(4*8)(r31)
	lfd	fp5,_UC_FREGS+(5*8)(r31)
	lfd	fp6,_UC_FREGS+(6*8)(r31)
	lfd	fp7,_UC_FREGS+(7*8)(r31)
	lfd	fp8,_UC_FREGS+(8*8)(r31)
	lfd	fp9,_UC_FREGS+(9*8)(r31)
	lfd	fp10,_UC_FREGS+(10*8)(r31)
	lfd	fp11,_UC_FREGS+(11*8)(r31)
	lfd	fp12,_UC_FREGS+(12*8)(r31)
	lfd	fp13,_UC_FREGS+(13*8)(r31)
	lfd	fp14,_UC_FREGS+(14*8)(r31)
	lfd	fp15,_UC_FREGS+(15*8)(r31)
	lfd	fp16,_UC_FREGS+(16*8)(r31)
	lfd	fp17,_UC_FREGS+(17*8)(r31)
	lfd	fp18,_UC_FREGS+(18*8)(r31)
	lfd	fp19,_UC_FREGS+(19*8)(r31)
	lfd	fp20,_UC_FREGS+(20*8)(r31)
	lfd	fp21,_UC_FREGS+(21*8)(r31)
	lfd	fp22,_UC_FREGS+(22*8)(r31)
	lfd	fp23,_UC_FREGS+(23*8)(r31)
	lfd	fp24,_UC_FREGS+(24*8)(r31)
	lfd	fp25,_UC_FREGS+(25*8)(r31)
	lfd	fp26,_UC_FREGS+(26*8)(r31)
	lfd	fp27,_UC_FREGS+(27*8)(r31)
	lfd	fp28,_UC_FREGS+(28*8)(r31)
	lfd	fp29,_UC_FREGS+(29*8)(r31)
	lfd	fp30,_UC_FREGS+(30*8)(r31)
	lfd	fp31,_UC_FREGS+(31*8)(r31)

	/* Restore LR and CCR, and set CTR to the NIP value */
	lwz	r3,_UC_GREGS+(PT_LNK*4)(r31)
	lwz	r4,_UC_GREGS+(PT_NIP*4)(r31)
	lwz	r5,_UC_GREGS+(PT_CCR*4)(r31)
	mtlr	r3
	mtctr	r4
	mtcr	r5

	/* Restore the general registers */
	lwz	r1,_UC_GREGS+(PT_R1*4)(r31)
	lwz	r3,_UC_GREGS+(PT_R3*4)(r31)
	lwz	r4,_UC_GREGS+(PT_R4*4)(r31)
	lwz	r5,_UC_GREGS+(PT_R5*4)(r31)
	lwz	r6,_UC_GREGS+(PT_R6*4)(r31)
	lwz	r7,_UC_GREGS+(PT_R7*4)(r31)
	lwz	r8,_UC_GREGS+(PT_R8*4)(r31)
	lwz	r9,_UC_GREGS+(PT_R9*4)(r31)
	lwz	r10,_UC_GREGS+(PT_R10*4)(r31)
	lwz	r11,_UC_GREGS+(PT_R11*4)(r31)
	lwz	r12,_UC_GREGS+(PT_R12*4)(r31)
	lwz	r13,_UC_GREGS+(PT_R13*4)(r31)
	lwz	r14,_UC_GREGS+(PT_R14*4)(r31)
	lwz	r15,_UC_GREGS+(PT_R15*4)(r31)
	lwz	r16,_UC_GREGS+(PT_R16*4)(r31)
	lwz	r17,_UC_GREGS+(PT_R17*4)(r31)
	lwz	r18,_UC_GREGS+(PT_R18*4)(r31)
	lwz	r19,_UC_GREGS+(PT_R19*4)(r31)
	lwz	r20,_UC_GREGS+(PT_R20*4)(r31)
	lwz	r21,_UC_GREGS+(PT_R21*4)(r31)
	lwz	r22,_UC_GREGS+(PT_R22*4)(r31)
	lwz	r23,_UC_GREGS+(PT_R23*4)(r31)
	lwz	r24,_UC_GREGS+(PT_R24*4)(r31)
	lwz	r25,_UC_GREGS+(PT_R25*4)(r31)
	lwz	r26,_UC_GREGS+(PT_R26*4)(r31)
	lwz	r27,_UC_GREGS+(PT_R27*4)(r31)
	lwz	r28,_UC_GREGS+(PT_R28*4)(r31)
	lwz	r29,_UC_GREGS+(PT_R29*4)(r31)
	lwz	r30,_UC_GREGS+(PT_R30*4)(r31)
	lwz	r31,_UC_GREGS+(PT_R31*4)(r31)

	bctr

L(error_exit):
	lwz	r31,12(r1)
	lwz	r0,20(r1)
	addi	r1,r1,16
	mtlr	r0
	blr

L(do_sigret):
	addi	r1,r3,-0xd0
	li	r0,SYS_ify(rt_sigreturn)
	sc
	/* NOTREACHED */

END (__setcontext)

versioned_symbol (libc, __setcontext, setcontext, GLIBC_2_3_4)

#if SHLIB_COMPAT (libc, GLIBC_2_3_3, GLIBC_2_3_4)

	compat_text_section
ENTRY(__novec_setcontext)
	mflr	r0
	stwu	r1,-16(r1)
	stw	r0,20(r1)
	stw	r31,12(r1)
	lwz	r31,_UC_REGS_PTR(r3)

	/*
	 * If this ucontext refers to the point where we were interrupted
	 * by a signal, we have to use the rt_sigreturn system call to
	 * return to the context so we get both LR and CTR restored.
	 *
	 * Otherwise, the context we are restoring is either just after
	 * a procedure call (getcontext/swapcontext) or at the beginning
	 * of a procedure call (makecontext), so we don't need to restore
	 * r0, xer, ctr.  We don't restore r2 since it will be used as
	 * the TLS pointer.
	 */
	lwz	r0,_UC_GREGS+(PT_MSR*4)(r31)
	cmpwi	r0,0
	bne	L(novec_do_sigret)

	/* Restore the signal mask */
	li	r5,0
	addi	r4,r3,_UC_SIGMASK
	li	r3,SIG_SETMASK
	bl	JUMPTARGET(__sigprocmask)
	cmpwi	r3,0
	bne	L(novec_error_exit)

	/* Restore the floating-point registers */
	lfd	fp31,_UC_FREGS+(32*8)(r31)
	lfd	fp0,_UC_FREGS+(0*8)(r31)
	mtfsf	0xff,fp31
	lfd	fp1,_UC_FREGS+(1*8)(r31)
	lfd	fp2,_UC_FREGS+(2*8)(r31)
	lfd	fp3,_UC_FREGS+(3*8)(r31)
	lfd	fp4,_UC_FREGS+(4*8)(r31)
	lfd	fp5,_UC_FREGS+(5*8)(r31)
	lfd	fp6,_UC_FREGS+(6*8)(r31)
	lfd	fp7,_UC_FREGS+(7*8)(r31)
	lfd	fp8,_UC_FREGS+(8*8)(r31)
	lfd	fp9,_UC_FREGS+(9*8)(r31)
	lfd	fp10,_UC_FREGS+(10*8)(r31)
	lfd	fp11,_UC_FREGS+(11*8)(r31)
	lfd	fp12,_UC_FREGS+(12*8)(r31)
	lfd	fp13,_UC_FREGS+(13*8)(r31)
	lfd	fp14,_UC_FREGS+(14*8)(r31)
	lfd	fp15,_UC_FREGS+(15*8)(r31)
	lfd	fp16,_UC_FREGS+(16*8)(r31)
	lfd	fp17,_UC_FREGS+(17*8)(r31)
	lfd	fp18,_UC_FREGS+(18*8)(r31)
	lfd	fp19,_UC_FREGS+(19*8)(r31)
	lfd	fp20,_UC_FREGS+(20*8)(r31)
	lfd	fp21,_UC_FREGS+(21*8)(r31)
	lfd	fp22,_UC_FREGS+(22*8)(r31)
	lfd	fp23,_UC_FREGS+(23*8)(r31)
	lfd	fp24,_UC_FREGS+(24*8)(r31)
	lfd	fp25,_UC_FREGS+(25*8)(r31)
	lfd	fp26,_UC_FREGS+(26*8)(r31)
	lfd	fp27,_UC_FREGS+(27*8)(r31)
	lfd	fp28,_UC_FREGS+(28*8)(r31)
	lfd	fp29,_UC_FREGS+(29*8)(r31)
	lfd	fp30,_UC_FREGS+(30*8)(r31)
	lfd	fp31,_UC_FREGS+(31*8)(r31)

	/* Restore LR and CCR, and set CTR to the NIP value */
	lwz	r3,_UC_GREGS+(PT_LNK*4)(r31)
	lwz	r4,_UC_GREGS+(PT_NIP*4)(r31)
	lwz	r5,_UC_GREGS+(PT_CCR*4)(r31)
	mtlr	r3
	mtctr	r4
	mtcr	r5

	/* Restore the general registers */
	lwz	r1,_UC_GREGS+(PT_R1*4)(r31)
	lwz	r3,_UC_GREGS+(PT_R3*4)(r31)
	lwz	r4,_UC_GREGS+(PT_R4*4)(r31)
	lwz	r5,_UC_GREGS+(PT_R5*4)(r31)
	lwz	r6,_UC_GREGS+(PT_R6*4)(r31)
	lwz	r7,_UC_GREGS+(PT_R7*4)(r31)
	lwz	r8,_UC_GREGS+(PT_R8*4)(r31)
	lwz	r9,_UC_GREGS+(PT_R9*4)(r31)
	lwz	r10,_UC_GREGS+(PT_R10*4)(r31)
	lwz	r11,_UC_GREGS+(PT_R11*4)(r31)
	lwz	r12,_UC_GREGS+(PT_R12*4)(r31)
	lwz	r13,_UC_GREGS+(PT_R13*4)(r31)
	lwz	r14,_UC_GREGS+(PT_R14*4)(r31)
	lwz	r15,_UC_GREGS+(PT_R15*4)(r31)
	lwz	r16,_UC_GREGS+(PT_R16*4)(r31)
	lwz	r17,_UC_GREGS+(PT_R17*4)(r31)
	lwz	r18,_UC_GREGS+(PT_R18*4)(r31)
	lwz	r19,_UC_GREGS+(PT_R19*4)(r31)
	lwz	r20,_UC_GREGS+(PT_R20*4)(r31)
	lwz	r21,_UC_GREGS+(PT_R21*4)(r31)
	lwz	r22,_UC_GREGS+(PT_R22*4)(r31)
	lwz	r23,_UC_GREGS+(PT_R23*4)(r31)
	lwz	r24,_UC_GREGS+(PT_R24*4)(r31)
	lwz	r25,_UC_GREGS+(PT_R25*4)(r31)
	lwz	r26,_UC_GREGS+(PT_R26*4)(r31)
	lwz	r27,_UC_GREGS+(PT_R27*4)(r31)
	lwz	r28,_UC_GREGS+(PT_R28*4)(r31)
	lwz	r29,_UC_GREGS+(PT_R29*4)(r31)
	lwz	r30,_UC_GREGS+(PT_R30*4)(r31)
	lwz	r31,_UC_GREGS+(PT_R31*4)(r31)

	bctr

L(novec_error_exit):
	lwz	r31,12(r1)
	lwz	r0,20(r1)
	addi	r1,r1,16
	mtlr	r0
	blr

L(novec_do_sigret):
	addi	r1,r3,-0xd0
	li	r0,SYS_ify(rt_sigreturn)
	sc
	/* NOTREACHED */

END (__novec_setcontext)
	.previous

compat_symbol (libc, __novec_setcontext, setcontext, GLIBC_2_3_3)

#endif

#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_3_3)

#define _ERRNO_H	1
#include <bits/errno.h>

	compat_text_section
ENTRY (__setcontext_stub)
	li	r3,ENOSYS
	b	JUMPTARGET(__syscall_error)
END (__setcontext_stub)
	.previous

compat_symbol (libc, __setcontext_stub, setcontext, GLIBC_2_0)

#endif
