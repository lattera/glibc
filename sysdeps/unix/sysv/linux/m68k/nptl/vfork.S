/* Copyright (C) 2010 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Maxim Kuvyrkov <maxim@codesourcery.com>, 2010.

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

#include <tcb-offsets.h>

#define SAVE_PID \
	bsrl	__m68k_read_tp@PLTPC	; /* Get the thread pointer.  */ \
	movel	%a0, %a1		; /* Save TP for RESTORE_PID.  */ \
	movel	PID_OFFSET(%a1), %d0	; /* Get the PID.  */ \
	movel	%d0, %d1		; /* Save PID for RESTORE_PID.  */ \
	negl	%d0			; /* Negate the PID.  */ \
	bne	1f			; /* If it was zero... */ \
	movel	#0x80000000, %d0	; /* use 0x80000000 instead.  */ \
1:	movel	%d0, PID_OFFSET(%a1)	; /* Store the temporary PID.  */

#define RESTORE_PID \
	tstl	%d0			; \
	beq	1f			; /* If we are the parent... */ \
	movel	%d1, PID_OFFSET(%a1)	; /* Restore the PID.  */ \
1:

#include <sysdeps/unix/sysv/linux/m68k/vfork.S>
