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
	stwu 1,-48(1)
#if NARGS >= 1
	stw 3,stackblock(1)
#endif
#if NARGS >= 2
	stw 4,4+stackblock(1)
#endif
#if NARGS >= 3
	stw 5,8+stackblock(1)
#endif
#if NARGS >= 4
	stw 6,12+stackblock(1)
#endif
#if NARGS >= 5
	stw 7,16+stackblock(1)
#endif
#if NARGS >= 6
	stw 8,20+stackblock(1)
#endif
#if NARGS >= 7
	stw 9,24+stackblock(1)
#endif
#if NARGS >= 8
	stw 10,28+stackblock(1)
#endif
#if NARGS >= 9
#error too many arguments!
#endif
	li 3,P(SOCKOP_,socket)
	addi 4,1,stackblock
	DO_CALL(SYS_ify(socketcall))
	addi 1,1,48
	bnslr
	b __syscall_error

PSEUDO_END (P(__,socket))

weak_alias (P(__,socket), socket)
