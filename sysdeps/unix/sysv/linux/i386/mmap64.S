/* Copyright (C) 1995, 96, 97, 98, 99, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sysdep.h>

#include "kernel-features.h"

#define EINVAL	22
#define ENOSYS	38

	.text

ENTRY (__mmap64)

#ifdef __NR_mmap2

	/* Save registers.  */
	pushl %ebp
	pushl %ebx
	pushl %esi
	pushl %edi

	movl 40(%esp), %edx
	movl 44(%esp), %ecx
	testl $0x3ff, %edx
	jne L(einval)
	shrdl $12, %ecx, %edx		/* mmap2 takes the offset in pages.  */
	shrl $12, %ecx
	jne L(einval)
	movl %edx, %ebp

	movl 20(%esp), %ebx
	movl 24(%esp), %ecx
	movl 28(%esp), %edx
	movl 32(%esp), %esi
	movl 36(%esp), %edi

	movl $SYS_ify(mmap2), %eax	/* System call number in %eax.  */

	/* Do the system call trap.  */
L(do_syscall):
	int $0x80

	/* Restore registers.  */
	popl %edi
	popl %esi
	popl %ebx
	popl %ebp

#ifndef __ASSUME_MMAP2_SYSCALL
2:
	cmp $-ENOSYS, %eax
	je 3f
#endif

	/* If 0 > %eax > -4096 there was an error.  */
	cmpl $-4096, %eax
	ja SYSCALL_ERROR_LABEL

	/* Successful; return the syscall's value.  */
L(pseudo_end):
	ret

	/* This means the offset value is too large.  */
L(einval):
	popl %edi
	popl %esi
	popl %ebx
	popl %ebp
	movl $-EINVAL, %eax
	jmp SYSCALL_ERROR_LABEL
#endif

#if !defined __ASSUME_MMAP2_SYSCALL || !defined __NR_mmap2

3:
	/* Save registers.  */
	movl %ebx, %edx

	cmpl $0, 28(%esp)
	jne L(einval)

	movl $SYS_ify(mmap), %eax	/* System call number in %eax.  */

	lea 4(%esp), %ebx		/* Address of args is 1st arg.  */

# ifdef __NR_mmap2
	jmp L(do_syscall)
# else

	/* Do the system call trap.  */
	int $0x80

	/* Restore registers.  */
	movl %edx, %ebx

	/* If 0 > %eax > -4096 there was an error.  */
	cmpl $-4096, %eax
	ja SYSCALL_ERROR_LABEL

	/* Successful; return the syscall's value.  */
L(pseudo_end):
	ret

L(einval):
	movl %edx, %ebx
	movl $-EINVAL, %eax
	jmp SYSCALL_ERROR_LABEL
# endif
#endif

PSEUDO_END (__mmap64)

weak_alias (__mmap64, mmap64)
