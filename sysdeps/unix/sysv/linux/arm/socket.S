/* Copyright (C) 1995, 1996, 1997, 1998, 2003, 2004, 2005
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

#include <sysdep-cancel.h>
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

#define PUSHARGS_1	str a1, [sp, $-4]!
#define PUSHARGS_2	stmfd sp!, {a1, a2}
#define PUSHARGS_3	stmfd sp!, {a1, a2, a3}
#define PUSHARGS_4	stmfd sp!, {a1, a2, a3, a4}
#define PUSHARGS_5	stmfd sp!, {a1, a2, a3, a4}	/* Caller has already pushed arg 5 */
#define PUSHARGS_6	stmfd sp!, {a1, a2, a3, a4}

#define POPARGS_1	add sp, sp, #4
#define POPARGS_2	add sp, sp, #8
#define POPARGS_3	add sp, sp, #12
#define POPARGS_4	add sp, sp, #16
#define POPARGS_5	add sp, sp, #16
#define POPARGS_6	add sp, sp, #16 

#ifndef NARGS
#define NARGS 3			/* If we were called with no wrapper, this is really socket() */
#endif

#if defined NEED_CANCELLATION && defined CENABLE
	PSEUDO_PROLOGUE
#endif

.globl __socket
ENTRY (__socket)
	/* This code previously moved sp into ip and stored the args using
	   stmdb ip!, {a1-a4}.  It did not modify sp, so the stack never had 
	   to be restored after the syscall completed.  It saved an 
	   instruction and meant no stack cleanup work was required.

	   This will not work in the case of a socket call being interrupted
	   by a signal.  If the signal handler uses any stack the arguments
	   to socket will be trashed.  The results of a restart of any
	   socket call are then unpredictable. */

	/* Push args onto the stack.  */
	P(PUSHARGS_,NARGS)

#if defined NEED_CANCELLATION && defined CENABLE
#ifdef PIC
	SINGLE_THREAD_P_PIC(r3)
#else
	SINGLE_THREAD_P
#endif
	bne 1f
#endif

        /* Do the system call trap.  */
	mov a1, $P(SOCKOP_,socket)
	mov a2, sp
	swi SYS_ify(socketcall)

	/* Pop args off the stack */
	P(POPARGS_,NARGS)

	/* r0 is < 0 if there was an error.  */
	cmn r0, $124
	RETINSTR(cc, r14)
	b PLTJMP(SYSCALL_ERROR)

#if defined NEED_CANCELLATION && defined CENABLE
1:
	str lr, [sp, #-4]!
	CENABLE
	mov ip, r0

	mov r0, #P(SOCKOP_,socket)
	add r1, sp, #4
	swi SYS_ify(socketcall)

	str r0, [sp, #-4]!
	mov r0, ip
	CDISABLE
	ldr r0, [sp], #4
	ldr lr, [sp], #4

	P(POPARGS_,NARGS)

	/* r0 is < 0 if there was an error.  */
	cmn r0, $124
	RETINSTR(cc, r14)
	b PLTJMP(SYSCALL_ERROR)
#endif

PSEUDO_END (__socket)

#ifndef NO_WEAK_ALIAS
weak_alias (__socket, socket)
#endif
