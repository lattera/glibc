/* longjmp for PowerPC.
   Copyright (C) 1995-1997, 1999-2001, 2003, 2004, 2005, 2006, 2009
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
#ifdef __NO_VMX__
# include <novmxsetjmp.h>
#else
# include <jmpbuf-offsets.h>
#endif
#include <bp-sym.h>
#include <bp-asm.h>

ENTRY (BP_SYM (__longjmp))
	CHECK_BOUNDS_BOTH_WIDE_LIT (r3, r8, r9, JB_SIZE)

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
	lwz r14,((JB_GPRS+0)*4)(r3)
	lwz r15,((JB_GPRS+1)*4)(r3)
	lwz r16,((JB_GPRS+2)*4)(r3)
	lwz r17,((JB_GPRS+3)*4)(r3)
	lwz r18,((JB_GPRS+4)*4)(r3)
	lwz r19,((JB_GPRS+5)*4)(r3)
	lwz r20,((JB_GPRS+6)*4)(r3)
#ifdef PTR_DEMANGLE
# ifndef CHECK_SP
	PTR_DEMANGLE3 (r1, r24, r25)
# endif
	PTR_DEMANGLE2 (r0, r25)
#endif
	mtlr r0
	lwz r21,((JB_GPRS+7)*4)(r3)
	lwz r22,((JB_GPRS+8)*4)(r3)
	lwz r0,(JB_CR*4)(r3)
	lwz r23,((JB_GPRS+9)*4)(r3)
	lwz r24,((JB_GPRS+10)*4)(r3)
	lwz r25,((JB_GPRS+11)*4)(r3)
	mtcrf 0xFF,r0
	lwz r26,((JB_GPRS+12)*4)(r3)
	lwz r27,((JB_GPRS+13)*4)(r3)
	lwz r28,((JB_GPRS+14)*4)(r3)
	lwz r29,((JB_GPRS+15)*4)(r3)
	lwz r30,((JB_GPRS+16)*4)(r3)
	lwz r31,((JB_GPRS+17)*4)(r3)
	mr r3,r4
	blr
END (BP_SYM (__longjmp))
