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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

#include "ucontext_i.h"

/* int swapcontext (ucontext_t *oucp, const ucontext_t *ucp) */

	.text
ENTRY (__swapcontext)
	FEEDBACK_ENTER(__swapcontext)
	/* Set up a frame and save r0 and r1.  */
	{
	 ST sp, lr
	 ADDI_PTR r11, sp, -(3 * REGSIZE)
	 move r10, sp
	}
	ADDI_PTR sp, sp, -(4 * REGSIZE)
	cfi_def_cfa_offset (4 * REGSIZE)
	cfi_offset (lr, 0)
	{
	 ST r11, r10
	 ADDI_PTR r10, sp, (2 * REGSIZE)
	}
	{
	 ST r10, r0
	 ADDI_PTR r10, sp, (3 * REGSIZE)
	}
	ST r10, r1

	/* Save the current context.  */
	jal __getcontext
	
	/* Tear down the frame and restore r0, r1, and lr. */
	{
	 BNEZ r0, .Lerror
	 ADDI_PTR r1, sp, 3 * REGSIZE
	}
	{
	 LD r1, r1
	 ADDI_PTR r0, sp, 2 * REGSIZE
	}
	{
	 LD r0, r0
	 ADDI_PTR sp, sp, 4 * REGSIZE
	}
	cfi_def_cfa_offset (0)
	{
	 LD lr, sp
	 ADDLI_PTR r10, r0, UC_REG(54)
	}

	/* Update the stored sp and lr.  */
	{
	 ST r10, sp
	 ADDLI_PTR r10, r0, UC_REG(55)
	}
	ST r10, lr

	/* Tail-call setcontext to finish up.  */
	{
	 move r0, r1
	 j __setcontext
	}

.Lerror:
	ADDI_PTR sp, sp, 4 * REGSIZE
	cfi_def_cfa_offset (0)
	LD lr, sp
	jrp lr
END (__swapcontext)

weak_alias (__swapcontext, swapcontext)
