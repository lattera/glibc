/* Copyright (C) 2003, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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

#define SYSOP_semtimedop 4

#define SVRSP	12		/* saved register space */
#define PARMS	LINKAGE+SVRSP	/* space for 3 saved regs */
#define SEMID	PARMS
#define SOPS	SEMID+4
#define NSOPS	SOPS+PTR_SIZE
#define TIMEOUT	NSOPS+4

	.text
ENTRY (BP_SYM (semtimedop))

	pushl	%ebp
	cfi_adjust_cfa_offset (4)
	pushl	%ebx
	cfi_adjust_cfa_offset (4)
	pushl	%edi
	cfi_adjust_cfa_offset (4)

	movl	$SYSOP_semtimedop, %ebx
	cfi_rel_offset (ebx, 4)
	movl	SEMID(%esp), %ecx
	movl	NSOPS(%esp), %edx
	movl	SOPS(%esp), %edi
	cfi_rel_offset (edi, 0)
	movl	TIMEOUT(%esp), %ebp
	cfi_rel_offset (ebp, 8)
	movl	$__NR_ipc, %eax

	ENTER_KERNEL

	/* Restore registers.  */
	popl	%edi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (edi)
	popl	%ebx
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebx)
	popl	%ebp
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebp)

	/* If 0 > %eax > -4096 there was an error.  */
	cmpl $-4096, %eax
	ja SYSCALL_ERROR_LABEL

	/* Successful; return the syscall's value.  */
L(pseudo_end):
	ret

#ifdef PIC
	.align	4
#endif
PSEUDO_END (BP_SYM (semtimedop))
