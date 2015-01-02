/* Copyright (C) 1995-2015 Free Software Foundation, Inc.
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

#include <sysdep-cancel.h>
#include <socketcall.h>
#include <tls.h>
#include <kernel-features.h>

#define EINVAL	22
#define ENOSYS	38

#ifdef __ASSUME_ACCEPT4_SOCKETCALL
# define errlabel SYSCALL_ERROR_LABEL
#else
# define errlabel .Lerr
	.data
have_accept4:
	.long	0
#endif

	.text
/* The socket-oriented system calls are handled unusally in Linux/i386.
   They are all gated through the single `socketcall' system call number.
   `socketcall' takes two arguments: the first is the subcode, specifying
   which socket function is being called; and the second is a pointer to
   the arguments to the specific function.  */

.globl __libc_accept4
ENTRY (__libc_accept4)
#ifdef CENABLE
	SINGLE_THREAD_P
	jne 1f
#endif

	/* Save registers.  */
	movl %ebx, %edx
	cfi_register (3, 2)

	movl $SYS_ify(socketcall), %eax	/* System call number in %eax.  */

	movl $SOCKOP_accept4, %ebx	/* Subcode is first arg to syscall.  */
	lea 4(%esp), %ecx		/* Address of args is 2nd arg.  */

        /* Do the system call trap.  */
	ENTER_KERNEL

	/* Restore registers.  */
	movl %edx, %ebx
	cfi_restore (3)

	/* %eax is < 0 if there was an error.  */
	cmpl $-125, %eax
	jae errlabel

	/* Successful; return the syscall's value.  */
	ret


#ifdef CENABLE
	/* We need one more register.  */
1:	pushl %esi
	cfi_adjust_cfa_offset(4)

	/* Enable asynchronous cancellation.  */
	CENABLE
	movl %eax, %esi
	cfi_offset(6, -8)		/* %esi */

	/* Save registers.  */
	movl %ebx, %edx
	cfi_register (3, 2)

	movl $SYS_ify(socketcall), %eax	/* System call number in %eax.  */

	movl $SOCKOP_accept4, %ebx	/* Subcode is first arg to syscall.  */
	lea 8(%esp), %ecx		/* Address of args is 2nd arg.  */

        /* Do the system call trap.  */
	ENTER_KERNEL

	/* Restore registers.  */
	movl %edx, %ebx
	cfi_restore (3)

	/* Restore the cancellation.  */
	xchgl %esi, %eax
	CDISABLE

	/* Restore registers.  */
	movl %esi, %eax
	popl %esi
	cfi_restore (6)
	cfi_adjust_cfa_offset(-4)

	/* %eax is < 0 if there was an error.  */
	cmpl $-125, %eax
	jae errlabel

	/* Successful; return the syscall's value.  */
	ret
#endif

#ifndef __ASSUME_ACCEPT4_SOCKETCALL
	/* The kernel returns -EINVAL for unknown socket operations.
	   We need to convert that error to an ENOSYS error.  */
.Lerr:	cmpl $-EINVAL, %eax
	jne SYSCALL_ERROR_LABEL

	/* Save registers.  */
	pushl %ebx
	cfi_adjust_cfa_offset(4)
	cfi_offset(ebx, -8)

# ifdef PIC
	SETUP_PIC_REG (dx)
	addl $_GLOBAL_OFFSET_TABLE_, %edx
	movl have_accept4@GOTOFF(%edx), %eax
# else
	movl have_accept4, %eax
# endif
	testl %eax, %eax
	jne 1f

	/* Try another call, this time with the FLAGS parameter
	   cleared and an invalid file descriptor.  This call will not
	   cause any harm and it will return immediately.  */
	movl $-1, 8(%esp)
	movl $0, 20(%esp)

	movl $SYS_ify(socketcall), %eax	/* System call number in %eax.  */

	movl $SOCKOP_accept4, %ebx	/* Subcode is first arg to syscall.  */
	lea 8(%esp), %ecx		/* Address of args is 2nd arg.  */

        /* Do the system call trap.  */
	ENTER_KERNEL

	cmpl $-EINVAL, %eax
	movl $-1, %eax
	je 3f
	movl $1, %eax
3:
# ifdef PIC
	movl %eax, have_accept4@GOTOFF(%edx)
# else
	movl %eax, have_accept4
# endif

	testl %eax, %eax

1:	movl $-EINVAL, %eax
	jns 2f
	movl $-ENOSYS, %eax

	/* Restore registers.  */
2:	popl %ebx
	cfi_restore (ebx)

	jmp SYSCALL_ERROR_LABEL
#endif
PSEUDO_END (__libc_accept4)

weak_alias (__libc_accept4, accept4)
