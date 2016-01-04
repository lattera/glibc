/* getcontext/setcontext/makecontext support for e500 high parts of registers.
   Copyright (C) 2006-2016 Free Software Foundation, Inc.
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

#ifndef _CONTEXT_E500_H
#define _CONTEXT_E500_H 1

#if defined __SPE__ || (defined __NO_FPRS__ && !defined _SOFT_FLOAT)

# define __CONTEXT_ENABLE_E500 1

/* We follow the kernel's layout, which saves the high parts of the
   SPE registers in the vregs area, immediately followed by the ACC
   value (call-clobbered, not handled here) and the SPEFSCR value.  */

.macro getcontext_e500
	la	r10,(_UC_VREGS)(r3)
	evstwwe	r0,(0*4)(r10)
	evstwwe	r1,(1*4)(r10)
	evstwwe	r2,(2*4)(r10)
	evstwwe	r3,(3*4)(r10)
	evstwwe	r4,(4*4)(r10)
	evstwwe	r5,(5*4)(r10)
	evstwwe	r6,(6*4)(r10)
	evstwwe	r7,(7*4)(r10)
	evstwwe	r8,(8*4)(r10)
	evstwwe	r9,(9*4)(r10)
	evstwwe	r10,(10*4)(r10)
	evstwwe	r11,(11*4)(r10)
	evstwwe	r12,(12*4)(r10)
	evstwwe	r13,(13*4)(r10)
	evstwwe	r14,(14*4)(r10)
	evstwwe	r15,(15*4)(r10)
	evstwwe	r16,(16*4)(r10)
	evstwwe	r17,(17*4)(r10)
	evstwwe	r18,(18*4)(r10)
	evstwwe	r19,(19*4)(r10)
	evstwwe	r20,(20*4)(r10)
	evstwwe	r21,(21*4)(r10)
	evstwwe	r22,(22*4)(r10)
	evstwwe	r23,(23*4)(r10)
	evstwwe	r24,(24*4)(r10)
	evstwwe	r25,(25*4)(r10)
	evstwwe	r26,(26*4)(r10)
	evstwwe	r27,(27*4)(r10)
	evstwwe	r28,(28*4)(r10)
	evstwwe	r29,(29*4)(r10)
	evstwwe	r30,(30*4)(r10)
	evstwwe	r31,(31*4)(r10)
	mfspefscr	r9
	stw	r9,(34*4)(r10)
.endm

.macro setcontext_e500
	lwz	r3,_UC_VREGS+(0*4)(r31)
	evmergelo	r0,r3,r0
	lwz	r3,_UC_VREGS+(1*4)(r31)
	evmergelo	r1,r3,r1
	lwz	r3,_UC_VREGS+(2*4)(r31)
	evmergelo	r2,r3,r2
	lwz	r3,_UC_VREGS+(1*4)(r31)
	evmergelo	r1,r3,r1
	lwz	r3,_UC_VREGS+(2*4)(r31)
	evmergelo	r2,r3,r2
	lwz	r3,_UC_VREGS+(3*4)(r31)
	evmergelo	r3,r3,r3
	lwz	r3,_UC_VREGS+(4*4)(r31)
	evmergelo	r4,r3,r4
	lwz	r3,_UC_VREGS+(5*4)(r31)
	evmergelo	r5,r3,r5
	lwz	r3,_UC_VREGS+(6*4)(r31)
	evmergelo	r6,r3,r6
	lwz	r3,_UC_VREGS+(7*4)(r31)
	evmergelo	r7,r3,r7
	lwz	r3,_UC_VREGS+(8*4)(r31)
	evmergelo	r8,r3,r8
	lwz	r3,_UC_VREGS+(9*4)(r31)
	evmergelo	r9,r3,r9
	lwz	r3,_UC_VREGS+(10*4)(r31)
	evmergelo	r10,r3,r10
	lwz	r3,_UC_VREGS+(11*4)(r31)
	evmergelo	r11,r3,r11
	lwz	r3,_UC_VREGS+(12*4)(r31)
	evmergelo	r12,r3,r12
	lwz	r3,_UC_VREGS+(13*4)(r31)
	evmergelo	r13,r3,r13
	lwz	r3,_UC_VREGS+(14*4)(r31)
	evmergelo	r14,r3,r14
	lwz	r3,_UC_VREGS+(15*4)(r31)
	evmergelo	r15,r3,r15
	lwz	r3,_UC_VREGS+(16*4)(r31)
	evmergelo	r16,r3,r16
	lwz	r3,_UC_VREGS+(17*4)(r31)
	evmergelo	r17,r3,r17
	lwz	r3,_UC_VREGS+(18*4)(r31)
	evmergelo	r18,r3,r18
	lwz	r3,_UC_VREGS+(19*4)(r31)
	evmergelo	r19,r3,r19
	lwz	r3,_UC_VREGS+(20*4)(r31)
	evmergelo	r20,r3,r20
	lwz	r3,_UC_VREGS+(21*4)(r31)
	evmergelo	r21,r3,r21
	lwz	r3,_UC_VREGS+(22*4)(r31)
	evmergelo	r22,r3,r22
	lwz	r3,_UC_VREGS+(23*4)(r31)
	evmergelo	r23,r3,r23
	lwz	r3,_UC_VREGS+(24*4)(r31)
	evmergelo	r24,r3,r24
	lwz	r3,_UC_VREGS+(25*4)(r31)
	evmergelo	r25,r3,r25
	lwz	r3,_UC_VREGS+(26*4)(r31)
	evmergelo	r26,r3,r26
	lwz	r3,_UC_VREGS+(27*4)(r31)
	evmergelo	r27,r3,r27
	lwz	r3,_UC_VREGS+(28*4)(r31)
	evmergelo	r28,r3,r28
	lwz	r3,_UC_VREGS+(29*4)(r31)
	evmergelo	r29,r3,r29
	lwz	r3,_UC_VREGS+(30*4)(r31)
	evmergelo	r30,r3,r30
	lwz	r3,_UC_VREGS+(31*4)(r31)
	evmergelo	r31,r3,r31
	lwz	r3,_UC_VREGS+(34*4)(r31)
	mtspefscr	r3
.endm
#else
# undef __CONTEXT_ENABLE_E500
#endif

#endif /* context-e500.h */
