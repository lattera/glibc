/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
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
#define VSYSCALL_ADDR_vgettimeofday	0xffffffffff600000


ENTRY (__gettimeofday)
	/* Align stack.  */
	sub	$0x8, %rsp
	cfi_adjust_cfa_offset(8)
	movq	$VSYSCALL_ADDR_vgettimeofday, %rax
	callq	*%rax
	/* Check error return.  */
	cmpl	$-4095, %eax
	jae	SYSCALL_ERROR_LABEL

L(pseudo_end):
	add	$0x8, %rsp
	cfi_adjust_cfa_offset(-8)
	ret
PSEUDO_END(__gettimeofday)

strong_alias (__gettimeofday, __gettimeofday_internal)
weak_alias (__gettimeofday, gettimeofday)
