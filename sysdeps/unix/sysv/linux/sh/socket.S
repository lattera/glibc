/* Copyright (C) 1999, 2000, 2003 Free Software Foundation, Inc.
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
#include <tls.h>

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

#define PUSHARGS_1	mov.l r4,@-r15
#define PUSHARGS_2	mov.l r5,@-r15; PUSHARGS_1
#define PUSHARGS_3	mov.l r6,@-r15; PUSHARGS_2
#define PUSHARGS_4	mov.l r7,@-r15; PUSHARGS_3
#define PUSHARGS_5	PUSHARGS_4	/* Caller has already pushed arg 5 */
#define PUSHARGS_6	PUSHARGS_4	/* Caller has already pushed arg 5,6 */

#define POPARGS_1	add #4,r15
#define POPARGS_2	add #8,r15
#define POPARGS_3	add #12,r15
#define POPARGS_4	add #16,r15
#define POPARGS_5	add #16,r15
#define POPARGS_6	add #16,r15 

#ifndef NARGS
#define NARGS 3			/* If we were called with no wrapper, this is really socket() */
#endif

.globl __socket
ENTRY (__socket)
	/* This will not work in the case of a socket call being interrupted
	   by a signal.  If the signal handler uses any stack the arguments
	   to socket will be trashed.  The results of a restart of any
	   socket call are then unpredictable. */

	/* Push args onto the stack.  */
	P(PUSHARGS_,NARGS)

#if defined NEED_CANCELLATION && defined CENABLE
	SINGLE_THREAD_P
	bf .Lsocket_cancel	
#endif

	/* Do the system call trap.  */
	mov #+P(SOCKOP_,socket), r4
	mov r15, r5
	mov.l .L1,r3
	trapa #0x12

	/* Pop args off the stack */
	P(POPARGS_,NARGS)

	mov     r0, r1
	mov	#-12, r2
	shad	r2, r1
	not	r1, r1			// r1=0 means r0 = -1 to -4095
	tst	r1, r1			// i.e. error in linux
	bf	.Lpseudo_end
.Lsyscall_error:
	SYSCALL_ERROR_HANDLER
.Lpseudo_end:
	/* Successful; return the syscall's value.  */
	rts
	 nop

#if defined NEED_CANCELLATION && defined CENABLE
.Lsocket_cancel:
	/* Enable asynchronous cancellation.  */
	CENABLE

	/* Do the system call trap.  */
	mov #+P(SOCKOP_,socket), r4
	mov r15, r5
	mov.l .L1,r3
	trapa #0x12

	mov.l r0,@-r15
	CDISABLE
	mov.l @r15+,r0

	/* Pop args off the stack */
	P(POPARGS_,NARGS)

	mov     r0, r1
	mov	#-12, r2
	shad	r2, r1
	not	r1, r1			// r1=0 means r0 = -1 to -4095
	tst	r1, r1			// i.e. error in linux
	bf	.Lpseudo_end
	bra	.Lsyscall_error
	 nop
#endif
	.align 2
.L1:
	.long	SYS_ify(socketcall)

PSEUDO_END (__socket)

weak_alias (__socket, socket)
