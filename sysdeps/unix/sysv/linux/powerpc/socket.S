/* Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
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
#include <sys/socketcall.h>

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
#ifdef socket
#error NARGS not defined
#endif
#define NARGS 3
#endif

#define stackblock 20

	.text
ENTRY(P(__,socket))
	stwu %r1,-48(%r1)
#if NARGS >= 1
	stw  %r3,stackblock(%r1)
#endif
#if NARGS >= 2
	stw  %r4,4+stackblock(%r1)
#endif
#if NARGS >= 3
	stw  %r5,8+stackblock(%r1)
#endif
#if NARGS >= 4
	stw  %r6,12+stackblock(%r1)
#endif
#if NARGS >= 5
	stw  %r7,16+stackblock(%r1)
#endif
#if NARGS >= 6
	stw  %r8,20+stackblock(%r1)
#endif
#if NARGS >= 7
	stw  %r9,24+stackblock(%r1)
#endif
#if NARGS >= 8
	stw  %r10,28+stackblock(%r1)
#endif
#if NARGS >= 9
#error too many arguments!
#endif
	li   %r3,P(SOCKOP_,socket)
	addi %r4,%r1,stackblock
	DO_CALL(SYS_ify(socketcall))
	addi %r1,%r1,48
	bnslr
#ifdef PIC
	b __syscall_error@plt
#else
	b __syscall_error
#endif

PSEUDO_END (P(__,socket))

weak_alias (P(__,socket), socket)
