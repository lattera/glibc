/* Copyright (C) 2000, 2001, 2003 Free Software Foundation, Inc.
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

#include <sysdep-cancel.h>
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
	cfi_offset (%r15, -36)
	cfi_offset (%r14, -40)
	cfi_offset (%r13, -44)
	cfi_offset (%r12, -48)
	cfi_offset (%r11, -52)
	cfi_offset (%r10, -56)
	cfi_offset (%r9, -60)
	cfi_offset (%r8, -64)
	cfi_offset (%r7, -68)
	cfi_offset (%r6, -72)
	lr      %r1,%r15
	l       %r0,4(0,%r15)           /* load eos */
	ahi     %r15,-120               /* buy stack space */
	cfi_adjust_cfa_offset (120)
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

#if defined NEED_CANCELLATION && defined CENABLE
	SINGLE_THREAD_P (%r4)
	jne	L(socket_cancel)
#endif

	/* load subcode for socket syscall */
	lhi     %r2,P(SOCKOP_,socket)
	la      %r3,0x60(0,%r15)        /* load address of parameter list */

	/* Do the system call trap.  */
	svc     SYS_ify(socketcall)

4:
	l       %r15,0(0,%r15)          /* load back chain */
	lm      %r6,15,24(%r15)         /* load registers */

	/* gpr2 is < 0 if there was an error.  */
	lhi     %r0,-125
	clr     %r2,%r0
	jnl     SYSCALL_ERROR_LABEL

	/* Successful; return the syscall's value.  */
	br      %r14

#if defined NEED_CANCELLATION && defined CENABLE
L(socket_cancel):
	basr	%r13,0
1:	l	%r1,2f-1b(%r13)
	/* call CENABLE.  */
	bas	%r14,0(%r13,%r1)
	lr	%r0,%r2

	/* load subcode for socket syscall */
	lhi     %r2,P(SOCKOP_,socket)
	la      %r3,0x60(0,%r15)        /* load address of parameter list */

	/* Do the system call trap.  */
	svc     SYS_ify(socketcall)

	l	%r3,3f-1b(%r13)
	lr	%r12,%r2
	lr	%r2,%r0
	/* call CDISABLE.  */
	bas     %r14,0(%r13,%r3)
	lr	%r2,%r12
	j	4b

2:	.long	CENABLE-1b
3:	.long	CDISABLE-1b
#endif

	SYSCALL_ERROR_HANDLER
END (__socket)

#ifndef NO_WEAK_ALIAS
weak_alias (__socket, socket)
#endif
