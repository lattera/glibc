/* setjmp for PowerPC64.
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

ENTRY (BP_SYM (__sigsetjmp))
	CHECK_BOUNDS_BOTH_WIDE_LIT (r3, r8, r9, JB_SIZE)
___sigsetjmp:
	std  r1,(JB_GPR1*8)(3)
	mflr r0
	std  r2,(JB_GPR2*8)(3)
	std  r14,((JB_GPRS+0)*8)(3)
	stfd fp14,((JB_FPRS+0)*8)(3)
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
	b JUMPTARGET (BP_SYM (__sigjmp_save))
END (BP_SYM (__sigsetjmp))

ENTRY (BP_SYM (_setjmp))
	li r4,0			/* Set second argument to 0.  */
	b	___sigsetjmp
END (BP_SYM (_setjmp))
libc_hidden_def (_setjmp)

ENTRY (BP_SYM (__setjmp))
	li r4,1			/* Set second argument to 1.  */
	b	___sigsetjmp
END (BP_SYM (__setjmp))

strong_alias (__setjmp, setjmp)
