/* Copyright (C) 1995,96,97,98,99,2000,2002,2005,2006
	Free Software Foundation, Inc.
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

#include <kernel-features.h>

#define EINVAL	22

	.text

ENTRY (__mmap)

/* I don't think it is worthwhile trying to use mmap2 whenever it
   is available.  Only use it when we are sure the syscall exists.  */
#ifdef __ASSUME_MMAP2_SYSCALL

	/* Save registers.  */
	pushl %ebp
	cfi_adjust_cfa_offset (4)
	pushl %ebx
	cfi_adjust_cfa_offset (4)
	pushl %esi
	cfi_adjust_cfa_offset (4)
	pushl %edi
	cfi_adjust_cfa_offset (4)

	movl 20(%esp), %ebx
	cfi_rel_offset (ebx, 8)
	movl 24(%esp), %ecx
	movl 28(%esp), %edx
	movl 32(%esp), %esi
	cfi_rel_offset (esi, 4)
	movl 36(%esp), %edi
	cfi_rel_offset (edi, 0)
	movl 40(%esp), %ebp
	cfi_rel_offset (ebp, 12)
	testl $0xfff, %ebp
	movl $-EINVAL, %eax
	jne L(skip)
	shrl $12, %ebp			/* mmap2 takes the offset in pages.  */

	movl $SYS_ify(mmap2), %eax	/* System call number in %eax.  */

	/* Do the system call trap.  */
	ENTER_KERNEL
L(skip):
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

#else

	/* Save registers.  */
	movl %ebx, %edx
	cfi_register (ebx, edx)

	movl $SYS_ify(mmap), %eax	/* System call number in %eax.  */

	lea 4(%esp), %ebx		/* Address of args is 1st arg.  */

	/* Do the system call trap.  */
	int $0x80

	/* Restore registers.  */
	movl %edx, %ebx
	cfi_restore (ebx)

#endif

	/* If 0 > %eax > -4096 there was an error.  */
	cmpl $-4096, %eax
	ja SYSCALL_ERROR_LABEL

	/* Successful; return the syscall's value.  */
L(pseudo_end):
	ret

PSEUDO_END (__mmap)

weak_alias (__mmap, mmap)
