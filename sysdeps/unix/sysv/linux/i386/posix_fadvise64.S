/* Copyright (C) 1995-2000,2002,2003,2004,2005,2006
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include <bp-sym.h>
#include <bp-asm.h>

#include <kernel-features.h>

#define EINVAL	22
#define ENOSYS	38
#define EOVERFLOW 75

#define SVRSP	16		/* saved register space */
#define PARMS	LINKAGE+SVRSP	/* space for 4 saved regs */
#define FD	PARMS
#define OFFLO	FD+4
#define OFFHI	OFFLO+4
#define LENLO	OFFHI+4
#define LENHI	LENLO+4
#define FLAGS	LENHI+4

	.text
ENTRY (BP_SYM (__posix_fadvise64_l64))

#ifdef __NR_fadvise64_64

	/* Save registers.  */
	pushl	%ebp
	cfi_adjust_cfa_offset (4)
	pushl	%ebx
	cfi_adjust_cfa_offset (4)
	pushl	%esi
	cfi_adjust_cfa_offset (4)
	pushl	%edi
	cfi_adjust_cfa_offset (4)

	movl	FD(%esp), %ebx
	cfi_rel_offset (ebx, 8)
	movl	OFFLO(%esp), %ecx
	movl	OFFHI(%esp), %edx
	movl	LENLO(%esp), %esi
	cfi_rel_offset (esi, 4)
	movl	LENHI(%esp), %edi
	cfi_rel_offset (edi, 0)
	movl	FLAGS(%esp), %ebp
	cfi_rel_offset (ebp, 12)

	movl	$SYS_ify(fadvise64_64), %eax
	ENTER_KERNEL

	/* Restore registers.  */
	popl	%edi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (edi)
	popl	%esi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (esi)
	popl	%ebx
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebx)
	popl	%ebp
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebp)

#ifndef __ASSUME_FADVISE64_64_SYSCALL
	cmpl	$-ENOSYS, %eax
	je	1f
#endif

	/* The function returns zero, or the error number.  So all we
	   have to do is negate the value passed back from the kernel.  */
	/* If 0 > %eax > -4096 there was an error.  */
	negl	%eax

	/* Successful; return the syscall's value.  */
	ret
#endif

#if defined __NR_fadvise64 \
    && (!defined __ASSUME_FADVISE64_64_SYSCALL || !defined __NR_fadvise64_64)
1:	/* Save registers.  */
	pushl	%ebx
	cfi_adjust_cfa_offset (4)
	pushl	%esi
	cfi_adjust_cfa_offset (4)
	pushl	%edi
	cfi_adjust_cfa_offset (4)

	/* Overflow check.  */
	cmpl	$0, LENHI(%esp)
	movl	$-EOVERFLOW, %eax
	jne	L(overflow)

	movl	FD(%esp), %ebx
	cfi_rel_offset (ebx, 8)
	movl	OFFLO(%esp), %ecx
	movl	OFFHI(%esp), %edx
	movl	LENLO(%esp), %esi
	cfi_rel_offset (esi, 4)
	movl	FLAGS(%esp), %edi
	cfi_rel_offset (edi, 0)

	movl	$SYS_ify(fadvise64), %eax
	ENTER_KERNEL

	/* Restore registers.  */
L(overflow):
	popl	%edi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (edi)
	popl	%esi
	cfi_adjust_cfa_offset (-4)
	cfi_restore (esi)
	popl	%ebx
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebx)

	/* If 0 > %eax > -4096 there was an error.  */
	negl	%eax

	/* Successful; return the syscall's value.  */
	ret
#else
	movl	$ENOSYS, %eax
	ret
#endif

END (BP_SYM (__posix_fadvise64_l64))

#ifdef __NR_fadvise64
	.section	.text.compat, "ax"
ENTRY (__posix_fadvise64_l32)
	DO_CALL (fadvise64, 5)
	negl %eax
	ret
PSEUDO_END_ERRVAL (__posix_fadvise64_l32)
#else
ENTRY (BP_SYM (__posix_fadvise64_l32))
	movl	$ENOSYS, %eax
	ret
END (BP_SYM (__posix_fadvise64_l32))
#endif

default_symbol_version (__posix_fadvise64_l64, posix_fadvise64, GLIBC_2.3.3)
symbol_version (__posix_fadvise64_l32, posix_fadvise64, GLIBC_2.2)
