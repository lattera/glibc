/* Copyright (C) 1995-2012 Free Software Foundation, Inc.
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
#include <bp-sym.h>
#include <bp-asm.h>

#include <kernel-features.h>

#define EINVAL	22
#define ENOSYS	38

#define SVRSP	16		/* saved register space */
#define PARMS	LINKAGE+SVRSP	/* space for 4 saved regs */
#define ADDR	PARMS
#define LEN	ADDR+PTR_SIZE
#define PROT	LEN+4
#define FLAGS	PROT+4
#define FD	FLAGS+4
#define OFFLO	FD+4
#define OFFHI	OFFLO+4

	.text
ENTRY (BP_SYM (__mmap64))

	/* Save registers.  */
	pushl %ebp
	cfi_adjust_cfa_offset (4)
	pushl %ebx
	cfi_adjust_cfa_offset (4)
	pushl %esi
	cfi_adjust_cfa_offset (4)
	pushl %edi
	cfi_adjust_cfa_offset (4)

	movl OFFLO(%esp), %edx
	movl OFFHI(%esp), %ecx
	testl $0xfff, %edx
	jne L(einval)
	shrdl $12, %ecx, %edx		/* mmap2 takes the offset in pages.  */
	shrl $12, %ecx
	jne L(einval)
	movl %edx, %ebp
	cfi_rel_offset (ebp, 12)

	movl ADDR(%esp), %ebx
	cfi_rel_offset (ebx, 8)
	movl LEN(%esp), %ecx
	movl PROT(%esp), %edx
	movl FLAGS(%esp), %esi
	cfi_rel_offset (esi, 4)
	movl FD(%esp), %edi
	cfi_rel_offset (edi, 0)

	movl $SYS_ify(mmap2), %eax	/* System call number in %eax.  */

	/* Do the system call trap.  */
L(do_syscall):
	ENTER_KERNEL

	/* Restore registers.  */
	popl %edi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (edi)
	popl %esi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (esi)
	popl %ebx
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebx)
	popl %ebp
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebp)

	/* If 0 > %eax > -4096 there was an error.  */
	cmpl $-4096, %eax
	ja SYSCALL_ERROR_LABEL

	/* Successful; return the syscall's value.  */
	ret

	cfi_adjust_cfa_offset (16)
	cfi_rel_offset (ebp, 12)
	cfi_rel_offset (ebx, 8)
	cfi_rel_offset (esi, 4)
	cfi_rel_offset (edi, 0)
	/* This means the offset value is too large.  */
L(einval):
	popl %edi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (edi)
	popl %esi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (esi)
	popl %ebx
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebx)
	popl %ebp
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebp)
	movl $-EINVAL, %eax
	jmp SYSCALL_ERROR_LABEL

PSEUDO_END (BP_SYM (__mmap64))

weak_alias (BP_SYM (__mmap64), BP_SYM (mmap64))
