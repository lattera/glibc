/* setjmp for ARM.
   Copyright (C) 1997-2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#define _SETJMP_H
#define _ASM
#include <bits/setjmp.h>
#include <rtld-global-offsets.h>
#include <arm-features.h>

ENTRY (__sigsetjmp)
	mov	ip, r0

	/* Save registers */
	stmia	ip!, {v1-v6, sl, fp, sp, lr}

#if !defined ARM_ASSUME_NO_IWMMXT || defined __SOFTFP__
# define NEED_HWCAP 1
#endif

#ifdef NEED_HWCAP
	/* Check if we have a VFP unit.  */
# ifdef IS_IN_rtld
	ldr	a3, 1f
	ldr	a4, .Lrtld_local_ro
0:	add	a3, pc, a3
	add	a3, a3, a4
	ldr	a3, [a3, #RTLD_GLOBAL_RO_DL_HWCAP_OFFSET]
# else
#  ifdef PIC
	ldr	a3, 1f
	ldr	a4, .Lrtld_global_ro
0:	add	a3, pc, a3
	ldr	a3, [a3, a4]
	ldr	a3, [a3, #RTLD_GLOBAL_RO_DL_HWCAP_OFFSET]
#  else
	ldr	a3, .Lhwcap
	ldr	a3, [a3, #0]
#  endif
# endif
#endif

#ifdef __SOFTFP__
	tst	a3, #HWCAP_ARM_VFP
	beq	.Lno_vfp
#endif

	/* Store the VFP registers.
	   Don't use VFP instructions directly because this code
	   is used in non-VFP multilibs.  */
	/* Following instruction is vstmia ip!, {d8-d15}.  */
	stc	p11, cr8, [ip], #64
	/* Store the floating-point status register.  */
	/* Following instruction is vmrs a4, fpscr.  */
	mrc	p10, 7, a4, cr1, cr0, 0
	str	a4, [ip], #4
.Lno_vfp:

#ifndef ARM_ASSUME_NO_IWMMXT
	tst	a3, #HWCAP_ARM_IWMMXT
	beq	.Lno_iwmmxt

	/* Save the call-preserved iWMMXt registers.  */
	/* Following instructions are wstrd wr10, [ip], #8 (etc.)  */
	stcl	p1, cr10, [r12], #8
	stcl	p1, cr11, [r12], #8
	stcl	p1, cr12, [r12], #8
	stcl	p1, cr13, [r12], #8
	stcl	p1, cr14, [r12], #8
	stcl	p1, cr15, [r12], #8
.Lno_iwmmxt:
#endif

	/* Make a tail call to __sigjmp_save; it takes the same args.  */
	B	PLTJMP(C_SYMBOL_NAME(__sigjmp_save))

#ifdef NEED_HWCAP
# ifdef IS_IN_rtld
1:	.long	_GLOBAL_OFFSET_TABLE_ - 0b - 8
.Lrtld_local_ro:
	.long	C_SYMBOL_NAME(_rtld_local_ro)(GOTOFF)
# else
#  ifdef PIC
1:	.long	_GLOBAL_OFFSET_TABLE_ - 0b - 8
.Lrtld_global_ro:
	.long	C_SYMBOL_NAME(_rtld_global_ro)(GOT)
#  else
.Lhwcap:
	.long	C_SYMBOL_NAME(_dl_hwcap)
#  endif
# endif
#endif

END (__sigsetjmp)

hidden_def (__sigsetjmp)
