/* longjmp for PowerPC64.
   Copyright (C) 1995, 1996, 1997, 1999, 2000, 2001, 2002 
   Free Software Foundation, Inc.
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
#include <bits/setjmp.h>
#include <bp-sym.h>
#include <bp-asm.h>

ENTRY (BP_SYM (__longjmp))
	CHECK_BOUNDS_BOTH_WIDE_LIT (r3, r8, r9, JB_SIZE)
	ld r1,(JB_GPR1*8)(r3)
	ld r2,(JB_GPR2*8)(r3)
	ld r0,(JB_LR*8)(r3)
	ld r14,((JB_GPRS+0)*8)(r3)
	lfd fp14,((JB_FPRS+0)*8)(r3)
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
	mtlr r0
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
