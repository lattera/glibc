/* Copyright (C) 1995, 1996, 1997, 1999, 2001 Free Software Foundation, Inc.
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

	.syntax no_register_prefix

#ifndef __socket
#define __socket P(__,socket)
#endif

ENTRY(__socket)
	subq	NARGS*4,sp

#if NARGS >= 2
	move.d	sp,r9
	move.d	r10,[r9+]
#else
	move.d	r10,[sp]
#endif
#if NARGS >= 2
	move.d	r11,[r9+]
#endif
#if NARGS >= 3
	move.d	r12,[r9+]
#endif
#if NARGS >= 4
	move.d	r13,[r9+]
#endif
#if NARGS >= 5
	move.d	[sp+NARGS*4],r13
	move.d	r13,[r9+]
#endif
#if NARGS >= 6
	move.d	[sp+NARGS*4+4],r13
	move.d	r13,[r9+]
#endif
#if NARGS >= 7
	move.d	[sp+NARGS*4+8],r13
	move.d	r13,[r9+]
#endif
#if NARGS >= 8
	move.d	[sp+NARGS*4+12],r13
	move.d	r13,[r9+]
#endif
#if NARGS >= 9
#error Too many arguments!
#endif
	moveq	P(SOCKOP_,socket),r10
	move.d	sp,r11
	movu.w	SYS_ify(socketcall),r9
	break	13
	cmps.w	-4096,r10
	bhs	0f
	addq	NARGS*4,sp
	Ret
	nop
PSEUDO_END (__socket)

weak_alias (__socket, socket)
