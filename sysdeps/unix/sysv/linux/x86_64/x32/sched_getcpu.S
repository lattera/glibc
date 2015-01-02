/* Copyright (C) 2012-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifdef SHARED
#include <sysdep.h>
#include <tls.h>
#define _ERRNO_H	1
#include <bits/errno.h>

ENTRY (sched_getcpu)
	/* Align stack and create local variable for result.  */
	sub	$0x8, %esp
	cfi_adjust_cfa_offset(8)

	mov	%esp, %edi
	xor	%esi, %esi
	mov	$VGETCPU_CACHE_OFFSET, %edx
	add	%fs:0, %edx

	call	__getcpu

	/* Local variable is result if the call is successful.  */
	mov	(%rsp), %edx
	/* Restore stack pointer before we might jump to
	   SYSCALL_ERROR_LABEL which returns to the caller.  */
	add	$0x8, %esp
	cfi_adjust_cfa_offset(-8)

	cmp	$-4095, %eax
	jae	SYSCALL_ERROR_LABEL

	mov	%edx, %eax
	ret
PSEUDO_END(sched_getcpu)
#endif
