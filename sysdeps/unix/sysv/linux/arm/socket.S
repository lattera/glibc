/* Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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
#define __socket P(__,socket)
#endif

#define PUSHARGS_1	stmfd ip!, {a1}
#define PUSHARGS_2	stmfd ip!, {a1, a2}
#define PUSHARGS_3	stmfd ip!, {a1, a2, a3}
#define PUSHARGS_4	stmfd ip!, {a1, a2, a3, a4}
#define PUSHARGS_5	stmfd ip!, {a1, a2, a3, a4}	/* Caller has already pushed arg 5 */
#define PUSHARGS_6	stmfd ip!, {a1, a2, a3, a4}

#ifndef NARGS
#define NARGS 3			/* If we were called with no wrapper, this is really socket() */
#endif

.globl __socket
ENTRY (__socket)
	/* Push args onto the stack.  */
	mov ip, sp
	P(PUSHARGS_,NARGS)

        /* Do the system call trap.  */
	mov a1, $P(SOCKOP_,socket)
	mov a2, ip
	swi SYS_ify(socketcall)

	/* r0 is < 0 if there was an error.  */
	cmn r0, $124
	bhs PLTJMP(syscall_error)

	/* Successful; return the syscall's value.  */
	ret

PSEUDO_END (__socket)

weak_alias (__socket, socket)
