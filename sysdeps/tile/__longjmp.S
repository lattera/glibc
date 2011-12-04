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
#include <jmpbuf-offsets.h>
#include <asm-syntax.h>
#include <arch/spr_def.h>

/* PL to return to via iret in longjmp */
#define RETURN_PL 0

	.text
ENTRY (__longjmp)
	FEEDBACK_ENTER(__longjmp)

#define RESTORE(r) { LD r, r0 ; ADDI_PTR r0, r0, REGSIZE }
	FOR_EACH_CALLEE_SAVED_REG(RESTORE)

	/* Make longjmp(buf, 0) return "1" instead.
	   At the same time, construct our iret context; we set ICS so
	   we can validly load EX_CONTEXT for iret without being
	   interrupted halfway through.  */
	{
	 LD r2, r0   /* retrieve ICS bit from jmp_buf */
	 movei r3, 1
	 CMPEQI r0, r1, 0
	}
	{
	 mtspr INTERRUPT_CRITICAL_SECTION, r3
	 shli r2, r2, SPR_EX_CONTEXT_0_1__ICS_SHIFT
	}
	{
	 mtspr EX_CONTEXT_0_0, lr
	 ori r2, r2, RETURN_PL
	}
	{
	 or r0, r1, r0
	 mtspr EX_CONTEXT_0_1, r2
	}
	iret
	jrp lr   /* Keep the backtracer happy. */
END (__longjmp)
