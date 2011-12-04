/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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
#include <arch/abi.h>

#include "ucontext_i.h"

/* int getcontext (ucontext_t *ucp) */

	.text
ENTRY (__getcontext)
	FEEDBACK_ENTER(__getcontext)

	/* Save the callee-saved GPRs.  There's no need to save the
	   caller-saved GPRs since the eventual setcontext() or
	   swapcontext() will assume those registers are all dead.
	   Save value "1" to uc_flags to later recognize getcontext().  */
	{ movei r11, 1; ADDI_PTR r10, r0, UC_FLAGS_OFFSET }
	{ ST r10, r11; addli r10, r0, UC_REG(30) }
	{ ST r10, r30; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r31; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r32; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r33; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r34; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r35; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r36; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r37; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r38; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r39; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r40; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r41; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r42; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r43; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r44; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r45; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r46; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r47; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r48; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r49; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r50; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r51; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, r52; ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, tp;  ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, sp;  ADDI_PTR r10, r10, REGSIZE }
	{ ST r10, lr;  ADDI_PTR r10, r10, REGSIZE }
	lnk r11   /* Point PC at the "jrp lr" instruction.  */
	addli r11, r11, .Lreturn - .
	{ ST r10, r11; ADDI_PTR r10, r10, REGSIZE }
	mfspr r11, INTERRUPT_CRITICAL_SECTION
	{
	 ST r10, r11
	 movei r1, 0
	}

	/* rt_sigprocmask (SIG_BLOCK, NULL, &ucp->uc_sigmask, _NSIG / 8) */
	{
	 movei r3, _NSIG / 8
	 addli r2, r0, UC_SIGMASK_OFFSET
	}
	{
	 movei r0, SIG_BLOCK
	 moveli TREG_SYSCALL_NR_NAME, __NR_rt_sigprocmask
	}
	swint1
	BNEZ r1, .Lsyscall_error

.Lreturn:
	{
	 movei r0, 0
	 jrp lr
	}

.Lsyscall_error:
	j SYSCALL_ERROR_NAME

END (__getcontext)
.hidden __getcontext

weak_alias (__getcontext, getcontext)
