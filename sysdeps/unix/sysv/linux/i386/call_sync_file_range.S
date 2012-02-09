/* Selective file content synch'ing.
   Copyright (C) 2006, 2007, 2011 Free Software Foundation, Inc.
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

#include <sysdep.h>
#define _ERRNO_H        1
#include <bits/errno.h>


	.text
ENTRY (__call_sync_file_range)
#ifdef __NR_sync_file_range
	pushl	%ebx
	cfi_adjust_cfa_offset (4)
	pushl	%esi
	cfi_adjust_cfa_offset (4)
	pushl	%edi
	cfi_adjust_cfa_offset (4)
	pushl	%ebp
	cfi_adjust_cfa_offset (4)

	movl	20(%esp), %ebx
	cfi_rel_offset (ebx, 12)
	movl	24(%esp), %ecx
	movl	28(%esp), %edx
	movl	32(%esp), %esi
	cfi_rel_offset (esi, 8)
	movl	36(%esp), %edi
	cfi_rel_offset (edi, 4)
	movl	40(%esp), %ebp
	cfi_rel_offset (ebp, 0)

	movl	$SYS_ify(sync_file_range), %eax
	ENTER_KERNEL

	popl	%ebp
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebp)
	popl	%edi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (edi)
	popl	%esi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (esi)
	popl	%ebx
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebx)

	cmpl	$-4095, %eax
	jae	SYSCALL_ERROR_LABEL
#else
	movl	$-ENOSYS, %eax
	jmp	SYSCALL_ERROR_LABEL
#endif
L(pseudo_end):
	ret
PSEUDO_END (__call_sync_file_range)
