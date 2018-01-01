/* setjmp for PowerPC.
   Copyright (C) 1995-2018 Free Software Foundation, Inc.
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
#include <stap-probe.h>
#define _ASM
#ifdef __NO_VMX__
# include <novmxsetjmp.h>
#else
# include <jmpbuf-offsets.h>
#endif

#if defined __SPE__ || (defined __NO_FPRS__ && !defined _SOFT_FLOAT)
# define SAVE_GP(N)	evstdd r##N,((JB_FPRS+((N)-14)*2)*4)(3)
#else
# define SAVE_GP(N)	stw r##N,((JB_GPRS+(N)-14)*4)(3)
#endif

ENTRY (__sigsetjmp_symbol)

#ifdef PTR_MANGLE
	mr   r5,r1
	PTR_MANGLE(r5, r10)
	stw  r5,(JB_GPR1*4)(3)
#else
	stw  r1,(JB_GPR1*4)(3)
#endif
	mflr r0
	/* setjmp probe expects longjmp first argument (4@3), second argument
	   (-4@4), and target address (4@0), respectively.  */
	LIBC_PROBE (setjmp, 3, 4@3, -4@4, 4@0)
	SAVE_GP (14)
#ifdef PTR_MANGLE
	PTR_MANGLE2 (r0, r10)
	li   r10,0
#endif
	stw  r0,(JB_LR*4)(3)
	SAVE_GP (15)
	mfcr r0
	SAVE_GP (16)
	stw  r0,(JB_CR*4)(3)
	SAVE_GP (17)
	SAVE_GP (18)
	SAVE_GP (19)
	SAVE_GP (20)
	SAVE_GP (21)
	SAVE_GP (22)
	SAVE_GP (23)
	SAVE_GP (24)
	SAVE_GP (25)
	SAVE_GP (26)
	SAVE_GP (27)
	SAVE_GP (28)
	SAVE_GP (29)
	SAVE_GP (30)
	SAVE_GP (31)
#if IS_IN (rtld)
	li   r3,0
	blr
#else
	b __sigjmp_save_symbol@local
#endif
END (__sigsetjmp_symbol)
