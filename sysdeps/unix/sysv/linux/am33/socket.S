/* Copyright 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Alexandre Oliva <aoliva@redhat.com>.
   Based on ../i386/socket.S.

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
#include <socketcall.h>

#define P(a, b) P2(a, b)
#define P2(a, b) a##b

	.text
/* The socket-oriented system calls are handled unusally in Linux.
   They are all gated through the single `socketcall' system call number.
   `socketcall' takes two arguments: the first is the subcode, specifying
   which socket function is being called; and the second is a pointer to
   the arguments to the specific function.

   The .S files for the other calls just #define socket and #include this.  */

#ifndef __socket
#ifndef NO_WEAK_ALIAS
#define __socket P(__,socket)
#else
#define __socket socket
#endif
#endif

.globl __socket
ENTRY (__socket)

	mov d0,(4,sp)
	mov d1,(8,sp)

	mov SYS_ify(socketcall),d0	/* System call number in d0.  */

	/* Use ## so `socket' is a separate token that might be #define'd.  */
	mov P(SOCKOP_,socket),a0	/* Subcode is first arg to syscall.  */
	mov sp,d1
	add 4,d1			/* Address of args is 2nd arg.  */

        /* Do the system call trap.  */
	syscall 0

	/* d0 is < 0 if there was an error.  */
	cmp -126,d0
	bls L(pseudo_end)
	jmp SYSCALL_ERROR_LABEL

	/* Successful; return the syscall's value.  */
L(pseudo_end):
	ret

PSEUDO_END (__socket)

#ifndef NO_WEAK_ALIAS
weak_alias (__socket, socket)
#endif
