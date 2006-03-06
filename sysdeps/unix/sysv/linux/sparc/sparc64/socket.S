/* Copyright (C) 1997, 1998, 2002, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Miguel de Icaza <miguel@gnu.ai.mit.edu>, 1997.

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

#ifndef NARGS
#ifdef socket
#error NARGS not defined
#endif
#define NARGS 3
#endif

	.text
/* The socket-oriented system calls are handled unusually in Linux.
   They are all gated through the single `socketcall' system call number.
   `socketcall' takes two arguments: the first is the subcode, specifying
   which socket function is being called; and the second is a pointer to
   the arguments to the specific function.

   The .S files for the other calls just #define socket and #include this.  */

#ifndef __socket
# ifndef NO_WEAK_ALIAS
#  define __socket P(__,socket)
# else
#  define __socket socket
# endif
#endif

	.globl	__syscall_error
ENTRY(__socket)

	/* Drop up to 6 arguments (recvfrom) into the memory allocated by
	   the caller for varargs, since that's really what we have.  */
	stx	%o0, [%sp + STACK_BIAS + 128 + 0]
	stx	%o1, [%sp + STACK_BIAS + 128 + 8]
#if NARGS > 2
	stx	%o2, [%sp + STACK_BIAS + 128 + 16]
#if NARGS > 3
	stx	%o3, [%sp + STACK_BIAS + 128 + 24]
#if NARGS > 4
	stx	%o4, [%sp + STACK_BIAS + 128 + 32]
#if NARGS > 5
	stx	%o5, [%sp + STACK_BIAS + 128 + 40]
#endif
#endif
#endif
#endif

#if defined NEED_CANCELLATION && defined CENABLE
	SINGLE_THREAD_P
	cmp	%g1, 0
	bne	.Lsocket_cancel
#endif
	 mov	P(SOCKOP_,socket), %o0		/* arg 1: socket subfunction */
	add	%sp, STACK_BIAS + 128, %o1	/* arg 2: parameter block */
	LOADSYSCALL(socketcall)
	ta	0x6d

	bcc,pt	%xcc, 1f
	 mov	%o7, %g1
	call	__syscall_error
	 mov	%g1, %o7
1:	retl
	 nop

#if defined NEED_CANCELLATION && defined CENABLE
.Lsocket_cancel:
	save	%sp, -160, %sp
	cfi_def_cfa_register (%fp)
	cfi_window_save
	cfi_register (%o7, %i7)
	CENABLE
	 nop
	mov	%o0, %l0
	add	%sp, 160 + STACK_BIAS + 128, %o1
	mov	P(SOCKOP_,socket), %o0
	LOADSYSCALL(socketcall)
	ta	0x6d

	bcc,pt	%xcc, 1f
	 mov	%o0, %l1
	CDISABLE;
	 mov	%l0, %o0;
	call	__syscall_error;
	 mov	%l1, %o0;
	ba,pt	%xcc, 2f
	 mov	-1, %l1;
1:	CDISABLE
	 mov	%l0, %o0
2:	jmpl	%i7 + 8, %g0
	 restore %g0, %l1, %o0
#endif

END(__socket)

#ifndef NO_WEAK_ALIAS
weak_alias (__socket, socket)
#endif
