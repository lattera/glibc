/* Copyright (C) 2000, 2001 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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
#include <socketcall.h>

/* &%/$&!! preprocessor */
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

#ifndef NARGS      /* the socket.o object is compiled directly ... */
#define NARGS 3
#endif

.globl __socket
ENTRY(__socket)

	/* Save registers and setup stack.  */
	stm     %r6,%r15,24(%r15)       /* save registers */
	lr      %r1,%r15
	l       %r0,4(0,%r15)           /* load eos */
	ahi     %r15,-120               /* buy stack space */
	st      %r1,0(0,%r15)           /* store back chain */
	st      %r0,4(0,%r15)           /* store eos */

	/* Reorder arguments */
#if (NARGS >= 6)
	mvc     0x74(4,%r15),216(%r15)  /* move between parameter lists */
#endif
#if (NARGS >= 5)
	st      %r6,0x70(0,%r15)        /* store into parameter list */
#endif
#if (NARGS >= 4)
	st      %r5,0x6C(0,%r15)        /* store into parameter list */
#endif
#if (NARGS >= 3)
	st      %r4,0x68(0,%r15)        /* store into parameter list */
#endif
#if (NARGS >= 2)
	st      %r3,0x64(0,%r15)        /* store into parameter list */
	st      %r2,0x60(0,%r15)
#endif
	/* load subcode for socket syscall */
	lhi     %r2,P(SOCKOP_,socket)
	la      %r3,0x60(0,%r15)        /* load address of parameter list */

	/* Do the system call trap.  */
	svc     SYS_ify(socketcall)

	l       %r15,0(0,%r15)          /* load back chain */
	lm      %r6,15,24(%r15)         /* load registers */

	/* gpr2 is < 0 if there was an error.  */
	lhi     %r0,-125
	clr     %r2,%r0
	jnl     SYSCALL_ERROR_LABEL

	/* Successful; return the syscall's value.  */
	br      %r14

PSEUDO_END (__socket)

#ifndef NO_WEAK_ALIAS
weak_alias (__socket, socket)
#endif
