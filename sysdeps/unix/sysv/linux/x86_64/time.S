/* Copyright (C) 2001,02, 2003 Free Software Foundation, Inc.
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
#define _ERRNO_H	1
#include <bits/errno.h>

/* For the calculation see asm/vsyscall.h.  */
#define VSYSCALL_ADDR_vtime	0xffffffffff600400


/* Return the current time as a `time_t' and also put it in *T if T is
   not NULL.  Time is represented as seconds from Jan 1 00:00:00 1970.  */

ENTRY (time)
	/* Align stack.  */
	sub	$0x8, %rsp
	cfi_adjust_cfa_offset(8)

	movq	$VSYSCALL_ADDR_vtime, %rax
	callq	*%rax

	add	$0x8, %rsp
	cfi_adjust_cfa_offset(-8)
	ret
PSEUDO_END_NOERRNO(time)
libc_hidden_def (time)
