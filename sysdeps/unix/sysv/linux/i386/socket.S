/* Copyright (C) 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <sysdep.h>
#include <sys/socketcall.h>

#define P(a, b) P2(a, b)
#define P2(a, b) a##b

.globl syscall_error

/* The socket-oriented system calls are handled unusally in Linux.
   They are all gated through the single `socketcall' system call number.
   `socketcall' takes two arguments: the first is the subcode, specifying
   which socket function is being called; and the second is a pointer to
   the arguments to the specific function.

   The .S files for the other calls just #define socket and #include this.  */

.globl P(__,socket)
ENTRY (P(__,socket))

	/* Save registers.  */
	movl %ebx, %edx

	movl $SYS_ify(socketcall), %eax	/* System call number in %eax.  */

	/* Use ## so `socket' is a separate token that might be #define'd.  */
	movl $P(SOCKOP_,socket), %ebx	/* Subcode is first arg to syscall.  */
	lea 8(%esp), %ecx		/* Address of args is 2nd arg.  */

        /* Do the system call trap.  */
	int $0x80

	/* Restore registers.  */
	movl %edx, %ebx

	/* %eax is < 0 if there was an error.  */
	testl %eax, %eax
	jl JUMPTARGET(syscall_error)

	/* Successful; return the syscall's value.  */
	ret

weak_alias (P(__,socket), socket)
