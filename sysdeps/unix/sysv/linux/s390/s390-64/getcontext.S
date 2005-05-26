/* Copyright (C) 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include <features.h>

#include "ucontext_i.h"

/*  __getcontext (const ucontext_t *ucp)

  Saves the machine context in UCP such that when it is activated,
  it appears as if __getcontext() returned again.

  This implementation is intended to be used for *synchronous* context
  switches only.  Therefore, it does not have to save anything
  other than the PRESERVED state.  */

ENTRY(__getcontext)
	lgr     %r5,%r2

	/* sigprocmask (SIG_BLOCK, NULL, &sc->sc_mask).  */
	la      %r2,SIG_BLOCK
	slgr	%r3,%r3
	la	%r4,SC_MASK(%r5)
	svc	SYS_ify(rt_sigprocmask)

	/* Store fpu context.  */
	stfpc   SC_FPC(%r5)
	std     %f0,SC_FPRS(%r5)
	std     %f1,SC_FPRS+8(%r5)
	std     %f2,SC_FPRS+16(%r5)
	std     %f3,SC_FPRS+24(%r5)
	std     %f4,SC_FPRS+32(%r5)
	std     %f5,SC_FPRS+40(%r5)
	std     %f6,SC_FPRS+48(%r5)
	std     %f7,SC_FPRS+56(%r5)
	std     %f8,SC_FPRS+64(%r5)
	std     %f9,SC_FPRS+72(%r5)
	std     %f10,SC_FPRS+80(%r5)
	std     %f11,SC_FPRS+88(%r5)
	std     %f12,SC_FPRS+96(%r5)
	std     %f13,SC_FPRS+104(%r5)
	std     %f14,SC_FPRS+112(%r5)
	std     %f15,SC_FPRS+120(%r5)

	/* Set __getcontext return value to 0.  */
	slgr    %r2,%r2

	/* Store access registers.  */
	stam    %a0,%a15,SC_ACRS(%r5)

	/* Store general purpose registers.  */
	stmg    %r0,%r15,SC_GPRS(%r5)

	/* Return.  */
	br	%r14
END(__getcontext)

weak_alias (__getcontext, getcontext)
