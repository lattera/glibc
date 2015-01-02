/* Copyright (C) 2009-2015 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep-cancel.h>
#include <socketcall.h>

#define P(a, b) P2(a, b)
#define P2(a, b) a##b

/* The socket-oriented system calls are handled unusally in Linux.
   They are all gated through the single `socketcall' system call number.
   `socketcall' takes two arguments: the first is the subcode, specifying
   which socket function is being called; and the second is a pointer to
   the arguments to the specific function.

   The .S files for the other calls just #define socket and #include this.
   They also #define a 'number-of-arguments' word in NARGS, which
   defaults to 3.  */

#ifndef NARGS
# ifdef socket
#  error NARGS not defined
# endif
# define NARGS 3
#endif

/* Stack usage:
   0 - r15 (return address)
   4 - 12 args to syscall
   16 - scratch for cancellation state
   20 - scratch for return value
*/
#define stackadjust 24

#ifndef __socket
# ifndef NO_WEAK_ALIAS
#  define __socket P(__,socket)
# else
#  define __socket socket
# endif
#endif

	.text
ENTRY(__socket)
	addik r1,r1,-stackadjust
	swi r15,r1,0
#if NARGS >= 1
	swi  r5,r1,stackadjust+4
#endif
#if NARGS >= 2
	swi  r6,r1,stackadjust+8
#endif
#if NARGS >= 3
	swi  r7,r1,stackadjust+12
#endif
#if NARGS >= 4
	swi  r8,r1,stackadjust+16
#endif
#if NARGS >= 5
	swi  r9,r1,stackadjust+20
#endif
#if NARGS >= 6
	swi  r10,r1,stackadjust+24
#endif
/* The rest of the args (if any) are on the caller's stack already.  */

#if defined NEED_CANCELLATION && defined CENABLE
	SINGLE_THREAD_P(r12)
	bnei	r12,L(socket_cancel)
#endif

	addik	r12,r0,SYS_ify(socketcall)
	addik	r5,r0,P(SOCKOP_,socket)
	addik	r6,r1,stackadjust+4
	brki	r14,8
	addk	r0,r0,r0
	lwi	r15,r1,0
	addik	r1,r1,stackadjust
	addik   r4,r0,-4095
	cmpu	r4,r4,r3
	bgei	r4,SYSCALL_ERROR_LABEL
	rtsd	r15,8
	addk	r0,r0,r0

#if defined NEED_CANCELLATION && defined CENABLE
L(socket_cancel):
	CENABLE
	swi	r3,r1,16
	addik	r12,r0,SYS_ify(socketcall)
	addik	r5,r0,P(SOCKOP_,socket)
	addik	r6,r1,stackadjust+4
	brki	r14,8
	addk	r0,r0,r0
	swi	r3,r1,20
	lwi	r5,r1,16
	CDISABLE
	lwi	r3,r1,20
	lwi	r15,r1,0
	addik	r1,r1,stackadjust
	addik   r4,r0,-4095
	cmpu	r4,r4,r3
	bgei	r4,SYSCALL_ERROR_LABEL
	rtsd	r15,8
	addk	r0,r0,r0
#endif

PSEUDO_END (__socket)

#ifndef NO_WEAK_ALIAS
weak_alias (__socket, socket)
#endif
