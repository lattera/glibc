/* longjmp for ARM.
   Copyright (C) 1997, 1998, 2005, 2006, 2009, 2010
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
#define _SETJMP_H
#define _ASM
#include <bits/setjmp.h>
#include <rtld-global-offsets.h>

/* __longjmp(jmpbuf, val) */

ENTRY (__longjmp)
	mov	ip, r0
	movs	r0, r1		/* get the return value in place */
	moveq	r0, #1		/* can't let setjmp() return zero! */

#ifdef CHECK_SP
	ldr	r4, [ip, #32]	/* jmpbuf's sp */
	cfi_undefined (r4)
	CHECK_SP (r4)
#endif
	LOADREGS(ia, ip!, {v1-v6, sl, fp, sp, lr})
	cfi_restore (v1)
	cfi_restore (v2)
	cfi_restore (v3)
	cfi_restore (v4)
	cfi_restore (v5)
	cfi_restore (v6)
	cfi_restore (sl)
	cfi_restore (fp)
	cfi_restore (sp)
	cfi_restore (lr)

#ifdef IS_IN_rtld
  	ldr	a2, 1f
	ldr	a3, Lrtld_local_ro
0:	add	a2, pc, a2
	add	a2, a2, a3
	ldr	a2, [a2, #RTLD_GLOBAL_RO_DL_HWCAP_OFFSET]
#else
#ifdef PIC
  	ldr	a2, 1f
	ldr	a3, Lrtld_global_ro
0:	add	a2, pc, a2
	ldr	a2, [a2, a3]
	ldr	a2, [a2, #RTLD_GLOBAL_RO_DL_HWCAP_OFFSET]
#else
	ldr	a2, Lhwcap
	ldr	a2, [a2, #0]
#endif
#endif

	tst	a2, #HWCAP_ARM_VFP
	beq	Lno_vfp

	/* Restore the VFP registers.  */
	/* Following instruction is vldmia ip!, {d8-d15}.  */
	ldc	p11, cr8, [r12], #64
	/* Restore the floating-point status register.  */
	ldr     a3, [ip], #4
	/* Following instruction is fmxr fpscr, a3.  */
	mcr	p10, 7, a3, cr1, cr0, 0
Lno_vfp:

	tst	a2, #HWCAP_ARM_IWMMXT
	beq	Lno_iwmmxt

	/* Restore the call-preserved iWMMXt registers.  */
	/* Following instructions are wldrd wr10, [ip], #8 (etc.)  */
	ldcl	p1, cr10, [r12], #8
	ldcl	p1, cr11, [r12], #8
	ldcl	p1, cr12, [r12], #8
	ldcl	p1, cr13, [r12], #8
	ldcl	p1, cr14, [r12], #8
	ldcl	p1, cr15, [r12], #8
Lno_iwmmxt:

	DO_RET(lr)

#ifdef IS_IN_rtld
1:	.long	_GLOBAL_OFFSET_TABLE_ - 0b - 8
Lrtld_local_ro:
	.long	C_SYMBOL_NAME(_rtld_local_ro)(GOTOFF)
#else
#ifdef PIC
1:	.long	_GLOBAL_OFFSET_TABLE_ - 0b - 8
Lrtld_global_ro:
	.long	C_SYMBOL_NAME(_rtld_global_ro)(GOT)
#else
Lhwcap:
	.long	C_SYMBOL_NAME(_dl_hwcap)
#endif
#endif

END (__longjmp)
