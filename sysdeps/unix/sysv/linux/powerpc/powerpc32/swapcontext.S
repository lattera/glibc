/* Save current context and jump to a new context.
   Copyright (C) 2002 Free Software Foundation, Inc.
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

#define __ASSEMBLY__
#include <asm/ptrace.h>
#include "ucontext_i.h"

ENTRY(__swapcontext)
	/* Save the current context */
	stw	r0,_UC_GREGS+(PT_R0*4)(r3)
	stw	r1,_UC_GREGS+(PT_R1*4)(r3)
	mflr	r0
	stwu	r1,-16(r1)
	stw	r0,20(r1)
	stw	r31,12(r1)
	stw	r31,_UC_GREGS+(PT_R31*4)(r3)
	mr	r31,r4			/* new context pointer */
	stw	r0,_UC_GREGS+(PT_LNK*4)(r3)
	stw	r0,_UC_GREGS+(PT_NIP*4)(r3)
	stw	r2,_UC_GREGS+(PT_R2*4)(r3)
	stw	r4,_UC_GREGS+(PT_R4*4)(r3)
	stw	r5,_UC_GREGS+(PT_R5*4)(r3)
	stw	r6,_UC_GREGS+(PT_R6*4)(r3)
	stw	r7,_UC_GREGS+(PT_R7*4)(r3)
	stw	r8,_UC_GREGS+(PT_R8*4)(r3)
	stw	r9,_UC_GREGS+(PT_R9*4)(r3)
	stw	r10,_UC_GREGS+(PT_R10*4)(r3)
	stw	r11,_UC_GREGS+(PT_R11*4)(r3)
	stw	r12,_UC_GREGS+(PT_R12*4)(r3)
	stw	r13,_UC_GREGS+(PT_R13*4)(r3)
	stw	r14,_UC_GREGS+(PT_R14*4)(r3)
	stw	r15,_UC_GREGS+(PT_R15*4)(r3)
	stw	r16,_UC_GREGS+(PT_R16*4)(r3)
	stw	r17,_UC_GREGS+(PT_R17*4)(r3)
	stw	r18,_UC_GREGS+(PT_R18*4)(r3)
	stw	r19,_UC_GREGS+(PT_R19*4)(r3)
	stw	r20,_UC_GREGS+(PT_R20*4)(r3)
	stw	r21,_UC_GREGS+(PT_R21*4)(r3)
	stw	r22,_UC_GREGS+(PT_R22*4)(r3)
	stw	r23,_UC_GREGS+(PT_R23*4)(r3)
	stw	r24,_UC_GREGS+(PT_R24*4)(r3)
	stw	r25,_UC_GREGS+(PT_R25*4)(r3)
	stw	r26,_UC_GREGS+(PT_R26*4)(r3)
	stw	r27,_UC_GREGS+(PT_R27*4)(r3)
	stw	r28,_UC_GREGS+(PT_R28*4)(r3)
	stw	r29,_UC_GREGS+(PT_R29*4)(r3)
	stw	r30,_UC_GREGS+(PT_R30*4)(r3)
	mfctr	r0
	stw	r0,_UC_GREGS+(PT_CTR*4)(r3)
	mfxer	r0
	stw	r0,_UC_GREGS+(PT_XER*4)(r3)
	mfcr	r0
	stw	r0,_UC_GREGS+(PT_CCR*4)(r3)

	/* Set the return value of swapcontext to "success".  R3 is the only 
	   register whose value is not preserved in the saved context.  */
	li	r0,0
	stw	r0,_UC_GREGS+(PT_R3*4)(r3)

	/* Zero fill fields that can't be set in user state. */
	stw	r0,_UC_GREGS+(PT_MSR*4)(r3)
	stw	r0,_UC_GREGS+(PT_MQ*4)(r3)

	/* Save the floating-point registers */
	stfd	fp0,_UC_FREGS+(0*8)(r3)
	stfd	fp1,_UC_FREGS+(1*8)(r3)
	stfd	fp2,_UC_FREGS+(2*8)(r3)
	stfd	fp3,_UC_FREGS+(3*8)(r3)
	stfd	fp4,_UC_FREGS+(4*8)(r3)
	stfd	fp5,_UC_FREGS+(5*8)(r3)
	stfd	fp6,_UC_FREGS+(6*8)(r3)
	stfd	fp7,_UC_FREGS+(7*8)(r3)
	stfd	fp8,_UC_FREGS+(8*8)(r3)
	stfd	fp9,_UC_FREGS+(9*8)(r3)
	stfd	fp10,_UC_FREGS+(10*8)(r3)
	stfd	fp11,_UC_FREGS+(11*8)(r3)
	stfd	fp12,_UC_FREGS+(12*8)(r3)
	stfd	fp13,_UC_FREGS+(13*8)(r3)
	stfd	fp14,_UC_FREGS+(14*8)(r3)
	stfd	fp15,_UC_FREGS+(15*8)(r3)
	stfd	fp16,_UC_FREGS+(16*8)(r3)
	stfd	fp17,_UC_FREGS+(17*8)(r3)
	stfd	fp18,_UC_FREGS+(18*8)(r3)
	stfd	fp19,_UC_FREGS+(19*8)(r3)
	stfd	fp20,_UC_FREGS+(20*8)(r3)
	stfd	fp21,_UC_FREGS+(21*8)(r3)
	stfd	fp22,_UC_FREGS+(22*8)(r3)
	stfd	fp23,_UC_FREGS+(23*8)(r3)
	stfd	fp24,_UC_FREGS+(24*8)(r3)
	stfd	fp25,_UC_FREGS+(25*8)(r3)
	stfd	fp26,_UC_FREGS+(26*8)(r3)
	stfd	fp27,_UC_FREGS+(27*8)(r3)
	stfd	fp28,_UC_FREGS+(28*8)(r3)
	stfd	fp29,_UC_FREGS+(29*8)(r3)
	mffs	fp0
	stfd	fp30,_UC_FREGS+(30*8)(r3)
	stfd	fp31,_UC_FREGS+(31*8)(r3)
	stfd	fp0,_UC_FREGS+(32*8)(r3)

	addi	r5,r3,_UC_SIGMASK
	addi	r4,r4,_UC_SIGMASK
	li	r3,SIG_SETMASK
	bl	JUMPTARGET(sigprocmask)
	cmpwi	r3,0
	bne	L(error_exit)

	/*
	 * If the new ucontext refers to the point where we were interrupted
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
	addi	r1,r31,-0xd0
	li	r0,SYS_ify(rt_sigreturn)
	sc
	/* NOTREACHED */

PSEUDO_END(__swapcontext)

weak_alias(__swapcontext, swapcontext)
