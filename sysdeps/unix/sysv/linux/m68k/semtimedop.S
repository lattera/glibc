/* Copyright (C) 2003, 2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Schwab <schwab@suse.de>, 2003.

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

#define SYSOP_semtimedop 4

#define SVRSP	8		/* saved register space */
#define PARMS	4+SVRSP		/* space for 3 saved regs */
#define SEMID	PARMS
#define SOPS	SEMID+4
#define NSOPS	SOPS+4
#define TIMEOUT	NSOPS+4

	.text
ENTRY (semtimedop)

	/* Save registers.  */
	move.l	%d2, %a1
	move.l	%d3, -(%sp)
	cfi_adjust_cfa_offset (4)
	move.l	%d5, -(%sp)
	cfi_adjust_cfa_offset (4)
	cfi_register (%d2, %a1)
	cfi_rel_offset (%d3, 0)
	cfi_rel_offset (%d5, 4)

	move.l	#SYSOP_semtimedop, %d1
	move.l	SEMID(%sp), %d2
	move.l	NSOPS(%sp), %d3
	move.l	SOPS(%sp), %d5
	move.l	TIMEOUT(%sp), %a0
	move.l	#SYS_ify (ipc), %d0

	trap	#0

	/* Restore registers.  */
	move.l	(%sp)+, %d5
	cfi_adjust_cfa_offset (-4)
	cfi_restore (%d5)
	move.l	(%sp)+, %d3
	cfi_adjust_cfa_offset (-4)
	cfi_restore (%d3)
	move.l	%a1, %d2
	cfi_restore (%d2)

	/* Check for error.  */
	tst.l	%d0
	jmi	SYSCALL_ERROR_LABEL

	/* Successful; return the syscall's value.  */
	ret

PSEUDO_END (semtimedop)
