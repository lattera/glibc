/* Copyright (C) 2011-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <asm/errno.h>
#include <arch/spr_def.h>
#include <arch/abi.h>

#include "ucontext_i.h"

/* PL to return to via iret in setcontext */
#define RETURN_PL 0

/* int setcontext (const ucontext_t *ucp) */

	.text
ENTRY (__setcontext)
	FEEDBACK_ENTER(__setcontext)

	/* See if this is a true signal context (flags == 0).
	   If so, restore by invoking rt_sigreturn().  */
#if UC_FLAGS_OFFSET != 0
# error "Add offset to r0 prior to load."
#endif
	LD_PTR r10, r0
	{
	 beqz r10, .Lsigreturn
	 addi r10, r10, -1  /* Confirm that it has value "1".  */
	}
	bnez r10, .Lbadcontext

	/* Save lr and r0 briefly on the stack and set the signal mask:
	   rt_sigprocmask (SIG_SETMASK, &ucp->uc_sigmask, NULL, _NSIG / 8).  */
	{
	 st sp, lr
	 ADDI_PTR r11, sp, -(2 * REGSIZE)
	 move r10, sp
	}
	ADDI_PTR sp, sp, -(3 * REGSIZE)
	cfi_def_cfa_offset (3 * REGSIZE)
	cfi_offset (lr, 0)
	{
	 st r11, r10
	 ADDI_PTR r10, sp, (2 * REGSIZE)
	}
	{
	 st r10, r0
	 ADDLI_PTR r1, r0, UC_SIGMASK_OFFSET
	}
	cfi_offset (r0, -REGSIZE)
	{
	 movei r3, _NSIG / 8
	 movei r2, 0
	}
	{
	 movei r0, SIG_SETMASK
	 moveli TREG_SYSCALL_NR_NAME, __NR_rt_sigprocmask
	}
	swint1
	ADDI_PTR r11, sp, 2 * REGSIZE  /* Restore uc_context to r11. */
	{
	 ld r11, r11
	 ADDI_PTR sp, sp, 3 * REGSIZE
	}
	cfi_def_cfa_offset (0)
	ld lr, sp
	{
	 ADDI_PTR r10, r11, UC_REG(0)
	 bnez r1, .Lsyscall_error
	}

	/* Restore the argument registers; note they will be random
	   unless makecontext() has been called.  */
	{ ld r0, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r1, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r2, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r3, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r4, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r5, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r6, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r7, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r8, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r9, r10; ADDLI_PTR r10, r10, UC_REG(30) - UC_REG(9) }

	/* Restore the callee-saved GPRs.  */
	{ ld r30, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r31, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r32, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r33, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r34, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r35, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r36, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r37, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r38, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r39, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r40, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r41, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r42, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r43, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r44, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r45, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r46, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r47, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r48, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r49, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r50, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r51, r10; ADDI_PTR r10, r10, REGSIZE }
	{ ld r52, r10; ADDI_PTR r10, r10, REGSIZE * 2 }
	/* Skip tp since it must not change for a given thread.  */
	{ ld sp, r10;  ADDI_PTR r10, r10, REGSIZE }
	{ ld lr, r10;  ADDI_PTR r10, r10, REGSIZE }
	{ ld r11, r10; ADDI_PTR r10, r10, REGSIZE }

	/* Construct an iret context; we set ICS so we can validly load
	   EX_CONTEXT for iret without being interrupted halfway through.  */
	{
	 ld r12, r10
	 movei r13, 1
	}
	{
	 mtspr INTERRUPT_CRITICAL_SECTION, r13
	 shli r12, r12, SPR_EX_CONTEXT_0_1__ICS_SHIFT
	}
	{
	 mtspr EX_CONTEXT_0_0, r11
	 ori r12, r12, RETURN_PL
	}
	mtspr EX_CONTEXT_0_1, r12
	iret
	jrp lr    /* keep the backtracer happy */

.Lsigreturn:
	/* This is a context obtained from a signal handler.
	   Perform a full restore by pushing the context
	   passed onto a simulated signal frame on the stack
	   and call the signal return syscall as if a signal
	   handler exited normally.  */
	{
	 ADDLI_PTR sp, sp, -(C_ABI_SAVE_AREA_SIZE + SI_MAX_SIZE + UC_SIZE)
	 ADDLI_PTR r1, sp, -UC_SIZE
	}
	cfi_def_cfa_offset (C_ABI_SAVE_AREA_SIZE + SI_MAX_SIZE + UC_SIZE)
	moveli r2, UC_SIZE / REGSIZE
0:      {
	 ld r10, r0
	 ADDI_PTR r0, r0, REGSIZE
	}
	{
	 st r1, r10
	 ADDI_PTR r1, r1, REGSIZE
	 addi r2, r2, -1
	}
	bnez r2, 0b
	moveli TREG_SYSCALL_NR_NAME, __NR_rt_sigreturn
	swint1

	/* Restore the stack and fall through to the error
	   path.  Successful rt_sigreturn never returns to
	   its calling place.  */
	ADDLI_PTR sp, sp, (C_ABI_SAVE_AREA_SIZE + SI_MAX_SIZE + UC_SIZE)
	cfi_def_cfa_offset (0)

.Lsyscall_error:
	j SYSCALL_ERROR_NAME

.Lbadcontext:
	{
	 movei r1, EINVAL
	 j SYSCALL_ERROR_NAME
	}

END (__setcontext)

.hidden __setcontext
weak_alias (__setcontext, setcontext)

ENTRY (__startcontext)
	cfi_undefined (lr)
	FEEDBACK_ENTER(__startcontext)
	jalr r31
	beqz r30, 1f
	{
	 move r0, r30
	 jal __setcontext
	}
1:	{
	 movei r0, 0
	 j HIDDEN_JUMPTARGET(exit)
	}
END (__startcontext)
.hidden __startcontext
