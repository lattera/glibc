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
#include <arch/abi.h>

	.text
ENTRY (syscall)
	FEEDBACK_ENTER(syscall)
	{ move TREG_SYSCALL_NR_NAME, r0; move r0, r1 }
	{ move r1, r2; move r2, r3 }
	{ move r3, r4; move r4, r5 }
	{ move r5, r6; move r6, r7 }
	swint1
	bnez r1, 0f
	jrp lr
PSEUDO_END (syscall)
