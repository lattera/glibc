/* Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sysdep.h>

/* Please consult the file sysdeps/unix/sysv/linux/arm/sysdep.h for
   more information about the value -4095 used below.*/

	.text
ENTRY (syscall)

	/* Normally encoding the system call number in the instruction is
	   good.  But we pay the price here.  */

	sub sp, sp, $0xc		@ get 3 words on the stack
	orr r0, r0, $0xef000000		@ make up a SWI instruction
	orr r0, r0, $SWI_BASE
	str r0, [sp]
	ldr r0, _reti
	str r0, [sp, $4]
	adr r0, _ret
	str r0, [sp, $8]
	mov r0, r1
	mov r1, r2
	mov r2, r3
	mov pc, sp
_ret:	add sp, sp, $0xc
	RETINSTR(mov, pc, r14)

_reti:	.word	0xe51ff004	@ ldr pc, [pc, $4]

PSEUDO_END (syscall)
