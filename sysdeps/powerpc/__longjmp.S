/* longjmp for PowerPC.
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sysdep.h>
#define _ASM
#include <jmp_buf.h>

ENTRY (__longjmp)
	lwz 1,(JB_GPR1*4)(3)
	lwz 2,(JB_GPR2*4)(3)
	lwz 0,(JB_LR*4)(3)
	lwz 14,((JB_GPRS+0)*4)(3)
	lfd 14,((JB_FPRS+0*2)*4)(3)
	lwz 15,((JB_GPRS+1)*4)(3)
	lfd 15,((JB_FPRS+1*2)*4)(3)
	lwz 16,((JB_GPRS+2)*4)(3)
	lfd 16,((JB_FPRS+2*2)*4)(3)
	lwz 17,((JB_GPRS+3)*4)(3)
	lfd 17,((JB_FPRS+3*2)*4)(3)
	lwz 18,((JB_GPRS+4)*4)(3)
	lfd 18,((JB_FPRS+4*2)*4)(3)
	lwz 19,((JB_GPRS+5)*4)(3)
	lfd 19,((JB_FPRS+5*2)*4)(3)
	lwz 20,((JB_GPRS+6)*4)(3)
	lfd 20,((JB_FPRS+6*2)*4)(3)
	mtlr 0
	lwz 21,((JB_GPRS+7)*4)(3)
	lfd 21,((JB_FPRS+7*2)*4)(3)
	lwz 22,((JB_GPRS+8)*4)(3)
	lfd 22,((JB_FPRS+8*2)*4)(3)
	lwz 23,((JB_GPRS+9)*4)(3)
	lfd 23,((JB_FPRS+9*2)*4)(3)
	lwz 24,((JB_GPRS+10)*4)(3)
	lfd 24,((JB_FPRS+10*2)*4)(3)
	lwz 25,((JB_GPRS+11)*4)(3)
	lfd 25,((JB_FPRS+11*2)*4)(3)
	lwz 26,((JB_GPRS+12)*4)(3)
	lfd 26,((JB_FPRS+12*2)*4)(3)
	lwz 27,((JB_GPRS+13)*4)(3)
	lfd 27,((JB_FPRS+13*2)*4)(3)
	lwz 28,((JB_GPRS+14)*4)(3)
	lfd 28,((JB_FPRS+14*2)*4)(3)
	lwz 29,((JB_GPRS+15)*4)(3)
	lfd 29,((JB_FPRS+15*2)*4)(3)
	lwz 30,((JB_GPRS+16)*4)(3)
	lfd 30,((JB_FPRS+16*2)*4)(3)
	lwz 31,((JB_GPRS+17)*4)(3)
	lfd 31,((JB_FPRS+17*2)*4)(3)
	blr
END (__longjmp)
