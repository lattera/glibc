/* longjmp for PowerPC.
   Copyright (C) 1995-2016 Free Software Foundation, Inc.
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
# define LOAD_GP(N)	evldd r##N,((JB_FPRS+((N)-14)*2)*4)(r3)
#else
# define LOAD_GP(N)	lwz r##N,((JB_GPRS+(N)-14)*4)(r3)
#endif

ENTRY (__longjmp_symbol)

#if defined PTR_DEMANGLE || defined CHECK_SP
	lwz r24,(JB_GPR1*4)(r3)
# ifdef CHECK_SP
#  ifdef PTR_DEMANGLE
	PTR_DEMANGLE3 (r24, r24, r25)
#  endif
	CHECK_SP (r24)
	mr r1,r24
# endif
#else
	lwz r1,(JB_GPR1*4)(r3)
#endif
	lwz r0,(JB_LR*4)(r3)
	LOAD_GP (14)
	LOAD_GP (15)
	LOAD_GP (16)
	LOAD_GP (17)
	LOAD_GP (18)
	LOAD_GP (19)
	LOAD_GP (20)
#ifdef PTR_DEMANGLE
# ifndef CHECK_SP
	PTR_DEMANGLE3 (r1, r24, r25)
# endif
	PTR_DEMANGLE2 (r0, r25)
#endif
	/* longjmp/longjmp_target probe expects longjmp first argument (4@3),
	   second argument (-4@4), and target address (4@0), respectively.  */
	LIBC_PROBE (longjmp, 3, 4@3, -4@4, 4@0)
	mtlr r0
	LOAD_GP (21)
	LOAD_GP (22)
	lwz r5,(JB_CR*4)(r3)
	LOAD_GP (23)
	LOAD_GP (24)
	LOAD_GP (25)
	mtcrf 0xFF,r5
	LOAD_GP (26)
	LOAD_GP (27)
	LOAD_GP (28)
	LOAD_GP (29)
	LOAD_GP (30)
	LOAD_GP (31)
	LIBC_PROBE (longjmp_target, 3, 4@3, -4@4, 4@0)
	mr r3,r4
	blr
END (__longjmp_symbol)
