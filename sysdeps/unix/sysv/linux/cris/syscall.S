/* Copyright (C) 2001 Free Software Foundation, Inc.
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

	.syntax no_register_prefix

/* Make syscall (callno, ...) into a system call.  */

ENTRY (syscall)
	move.d	r10,r9
	move.d	r11,r10
	move.d	r12,r11
	move.d	r13,r12
	move.d	[sp],r13
	move	srp,[sp]
	move	[sp+4],mof
	move	[sp+8],srp
	break	13
	cmps.w	-4096,r10
	bhs	0f
	move	[sp],srp
	Ret
	nop
PSEUDO_END (syscall)
