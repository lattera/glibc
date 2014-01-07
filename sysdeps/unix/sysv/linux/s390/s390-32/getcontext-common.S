/* Copyright (C) 2001-2014 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).

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
#include <features.h>

#include "rtld-global-offsets.h"
#include "ucontext_i.h"

/*  __getcontext (const ucontext_t *ucp)

  Saves the machine context in UCP such that when it is activated,
  it appears as if __getcontext() returned again.

  This implementation is intended to be used for *synchronous* context
  switches only.  Therefore, it does not have to save anything
  other than the PRESERVED state.  */

ENTRY(__getcontext)
	lr      %r1,%r2

	/* rt_sigprocmask (SIG_BLOCK, NULL, &sc->sc_mask, sigsetsize).  */
	la      %r2,SIG_BLOCK
	slr	%r3,%r3
	la      %r4,SC_MASK(%r1)
	lhi	%r5,_NSIG8
	svc	SYS_ify(rt_sigprocmask)

	/* Store fpu context.  */
	stfpc   SC_FPC(%r1)
	std     %f0,SC_FPRS(%r1)
	std     %f1,SC_FPRS+8(%r1)
	std     %f2,SC_FPRS+16(%r1)
	std     %f3,SC_FPRS+24(%r1)
	std     %f4,SC_FPRS+32(%r1)
	std     %f5,SC_FPRS+40(%r1)
	std     %f6,SC_FPRS+48(%r1)
	std     %f7,SC_FPRS+56(%r1)
	std     %f8,SC_FPRS+64(%r1)
	std     %f9,SC_FPRS+72(%r1)
	std     %f10,SC_FPRS+80(%r1)
	std     %f11,SC_FPRS+88(%r1)
	std     %f12,SC_FPRS+96(%r1)
	std     %f13,SC_FPRS+104(%r1)
	std     %f14,SC_FPRS+112(%r1)
	std     %f15,SC_FPRS+120(%r1)

	lhi	%r2,0
#ifndef __V1_UCONTEXT
	bras	%r3,0f
# ifdef IS_IN_rtld
  /* Within ld.so we can do slightly better by addressing dl_hwap
     relative to GOT start.  */
1:	.long	_GLOBAL_OFFSET_TABLE_ - 1b
	.long	C_SYMBOL_NAME(_rtld_global_ro)@GOTOFF
0:	l	%r4,0(%r3)
	la	%r4,0(%r3,%r4)
	l	%r5,4(%r3)
	/* _dl_hwcap is 64 bit and we need the lower 32.  */
	l	%r3,RTLD_GLOBAL_RO_DL_HWCAP_OFFSET+4(%r4,%r5)
# elif PIC
1:	.long	_GLOBAL_OFFSET_TABLE_ - 1b
	.long	C_SYMBOL_NAME(_rtld_global_ro)@GOT
0:	l	%r4,0(%r3)
	la	%r4,0(%r3,%r4)  /* GOT pointer -> r4 */
	l	%r5,4(%r3)      /* GOT offset -> r5 */
	l	%r5,0(%r4,%r5)  /* GOT slot -> r5 */
	l	%r3,RTLD_GLOBAL_RO_DL_HWCAP_OFFSET+4(%r5)
# else
	.long	C_SYMBOL_NAME(_dl_hwcap)
0:	l	%r3,0(%r3)
	l	%r3,0(%r3)
# endif
	tml	%r3,512 /* HWCAP_S390_HIGH_GPRS */
	jz	2f
	/* highgprs implies zarch so stmh/oill is ok here.  */
	.machine "z900"
	.machinemode "zarch_nohighgprs"
	stmh	%r0,%r15,SC_HIGHGPRS(%r1)
	oill	%r2,1 /* UCONTEXT_UC_FLAGS_HIGH_GPRS */
#endif
2:	st	%r2,SC_FLGS(%r1)

	/* Set __getcontext return value to 0.  */
	slr     %r2,%r2

	/* Store access registers.  */
	stam    %a0,%a15,SC_ACRS(%r1)

	/* Store general purpose registers.  */
	stm     %r0,%r15,SC_GPRS(%r1)

	/* Return.  */
	br	%r14
END(__getcontext)

weak_alias (__getcontext, getcontext)
